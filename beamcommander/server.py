"""
Main BeamCommander server application
"""
import logging
import time
import threading
from flask import Flask, render_template, jsonify, send_from_directory
from flask_cors import CORS
import os
import sys

from .app_state import AppState
from .osc_receiver import OSCReceiver
from .shapes import ShapeGenerator
from .cue_manager import CueManager

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class BeamCommanderServer:
    """
    Main server application for BeamCommander
    """
    
    def __init__(self, osc_port: int = 9000, http_port: int = 8080):
        """
        Initialize BeamCommander server
        
        Args:
            osc_port: UDP port for OSC messages (default: 9000)
            http_port: HTTP port for web interface (default: 8080)
        """
        self.osc_port = osc_port
        self.http_port = http_port
        
        # Initialize components
        self.state = AppState()
        self.shape_generator = ShapeGenerator()
        self.cue_manager = CueManager(self.state)
        
        # Initialize OSC receiver
        self.osc_receiver = OSCReceiver(self.state, osc_port)
        self.osc_receiver.on_cue_save = self.cue_manager.save_cue
        self.osc_receiver.on_cue_recall = self.cue_manager.recall_cue
        
        # Flask app for web interface
        self.app = Flask(__name__,
                        static_folder='static',
                        template_folder='templates')
        CORS(self.app)
        self._setup_routes()
        
        # Animation state
        self.running = False
        self.start_time = time.time()
        
        # Load cues from disk
        self.cue_manager.load_from_disk()
        
        logger.info("BeamCommander server initialized")
    
    def _setup_routes(self):
        """Setup Flask routes for web interface"""
        
        @self.app.route('/')
        def index():
            """Serve main web interface"""
            return render_template('index.html')
        
        @self.app.route('/api/state')
        def get_state():
            """Get current application state"""
            return jsonify(self.state.to_dict())
        
        @self.app.route('/api/shapes')
        def get_shapes():
            """Get current shape points"""
            current_time = time.time() - self.start_time
            points = self.shape_generator.generate_shape(self.state, current_time)
            return jsonify({
                'points': points,
                'blackout': self.state.blackout
            })
        
        @self.app.route('/api/cues')
        def get_cues():
            """Get all cues"""
            cues_data = {}
            for num, cue in self.cue_manager.cues.items():
                if cue.populated:
                    cues_data[num] = cue.to_dict()
            return jsonify(cues_data)
        
        @self.app.route('/api/status')
        def get_status():
            """Get server status"""
            return jsonify({
                'running': self.running,
                'osc_port': self.osc_port,
                'uptime': time.time() - self.start_time
            })
    
    def start(self):
        """Start the BeamCommander server"""
        if self.running:
            logger.warning("Server already running")
            return
        
        logger.info("Starting BeamCommander server...")
        self.running = True
        self.start_time = time.time()
        
        # Start OSC receiver
        self.osc_receiver.start()
        logger.info(f"OSC receiver listening on port {self.osc_port}")
        
        # Start Flask app
        logger.info(f"Starting web interface on http://0.0.0.0:{self.http_port}")
        logger.info("=" * 60)
        logger.info("BeamCommander is ready!")
        logger.info(f"  Web Interface: http://localhost:{self.http_port}")
        logger.info(f"  OSC Port: {self.osc_port}")
        logger.info("=" * 60)
        
        # Run Flask in main thread
        self.app.run(host='0.0.0.0', port=self.http_port, debug=False, threaded=True)
    
    def stop(self):
        """Stop the BeamCommander server"""
        if not self.running:
            return
        
        logger.info("Stopping BeamCommander server...")
        self.running = False
        
        # Save cues before stopping
        self.cue_manager.save_to_disk()
        
        # Stop OSC receiver
        self.osc_receiver.stop()
        
        logger.info("BeamCommander server stopped")


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description='BeamCommander Laser Control Server')
    parser.add_argument('--osc-port', type=int, default=9000,
                       help='OSC receiver port (default: 9000)')
    parser.add_argument('--http-port', type=int, default=8080,
                       help='HTTP server port for web interface (default: 8080)')
    parser.add_argument('--log-level', default='INFO',
                       choices=['DEBUG', 'INFO', 'WARNING', 'ERROR'],
                       help='Logging level (default: INFO)')
    
    args = parser.parse_args()
    
    # Set logging level
    logging.getLogger().setLevel(getattr(logging, args.log_level))
    
    # Create and start server
    server = BeamCommanderServer(osc_port=args.osc_port, http_port=args.http_port)
    
    try:
        server.start()
    except KeyboardInterrupt:
        logger.info("\nReceived interrupt signal")
        server.stop()
    except Exception as e:
        logger.error(f"Server error: {e}", exc_info=True)
        server.stop()
        sys.exit(1)


if __name__ == '__main__':
    main()
