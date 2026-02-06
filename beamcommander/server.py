"""
Main BeamCommander server application
"""
import logging
import time
import threading
from typing import Optional
from flask import Flask, render_template, jsonify, send_from_directory
from flask_cors import CORS
import os
import sys

from .app_state import AppState
from .osc_receiver import OSCReceiver
from .shapes import ShapeGenerator
from .cue_manager import CueManager
from .etherdream import DACOutputManager

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
    
    def __init__(self, osc_port: int = 9000, http_port: int = 8080, enable_dac: bool = False, dac_ip: Optional[str] = None):
        """
        Initialize BeamCommander server
        
        Args:
            osc_port: UDP port for OSC messages (default: 9000)
            http_port: HTTP port for web interface (default: 8080)
            enable_dac: Enable EtherDream DAC output (default: False)
            dac_ip: IP address of DAC (None = auto-discover)
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
        
        # Initialize DAC output manager
        self.dac_manager = DACOutputManager(dac_ip=dac_ip, enable=enable_dac)
        self.enable_dac = enable_dac
        
        # DAC output thread
        self.dac_thread: Optional[threading.Thread] = None
        self.dac_running = False
        
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
        if enable_dac:
            logger.info("EtherDream DAC output ENABLED")
    
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
        
        @self.app.route('/api/osc', methods=['POST'])
        def send_osc():
            """Send OSC command from web UI"""
            from flask import request
            data = request.get_json()
            if not data or 'address' not in data:
                return jsonify({'error': 'Invalid request'}), 400
            
            address = data['address']
            args = data.get('args', [])
            
            # Simulate OSC message by calling the appropriate handler
            try:
                # Find the handler for this address
                if address == '/laser/shape' and args:
                    self.osc_receiver._handle_shape(address, *args)
                elif address == '/laser/color':
                    self.osc_receiver._handle_color(address, *args)
                elif address == '/laser/brightness' and args:
                    self.osc_receiver._handle_brightness(address, *args)
                elif address == '/laser/dotted' and args:
                    self.osc_receiver._handle_dotted(address, *args)
                elif address == '/laser/flicker' and args:
                    self.osc_receiver._handle_flicker(address, *args)
                elif address == '/laser/position':
                    self.osc_receiver._handle_position(address, *args)
                elif address == '/laser/position/x' and args:
                    self.osc_receiver._handle_position_x(address, *args)
                elif address == '/laser/position/y' and args:
                    self.osc_receiver._handle_position_y(address, *args)
                elif address == '/laser/shape/scale' and args:
                    self.osc_receiver._handle_scale(address, *args)
                elif address == '/laser/rotation/speed' and args:
                    self.osc_receiver._handle_rotation_speed(address, *args)
                elif address == '/laser/wave/frequency' and args:
                    self.osc_receiver._handle_wave_frequency(address, *args)
                elif address == '/laser/wave/amplitude' and args:
                    self.osc_receiver._handle_wave_amplitude(address, *args)
                elif address == '/laser/wave/speed' and args:
                    self.osc_receiver._handle_wave_speed(address, *args)
                elif address == '/laser/rainbow/amount' and args:
                    self.osc_receiver._handle_rainbow_amount(address, *args)
                elif address == '/laser/rainbow/speed' and args:
                    self.osc_receiver._handle_rainbow_speed(address, *args)
                elif address == '/laser/rainbow/blend' and args:
                    self.osc_receiver._handle_rainbow_blend(address, *args)
                elif address == '/move/mode' and args:
                    self.osc_receiver._handle_move_mode(address, *args)
                elif address == '/move/size' and args:
                    self.osc_receiver._handle_move_size(address, *args)
                elif address == '/move/speed' and args:
                    self.osc_receiver._handle_move_speed(address, *args)
                elif address == '/flash' and args:
                    self.osc_receiver._handle_flash(address, *args)
                elif address == '/blackout' and args:
                    self.osc_receiver._handle_blackout(address, *args)
                else:
                    return jsonify({'error': 'Unknown OSC address'}), 400
                
                return jsonify({'success': True})
            except Exception as e:
                logger.error(f"Error handling OSC command: {e}")
                return jsonify({'error': str(e)}), 500
    
    def _dac_output_loop(self):
        """DAC output thread - sends points to laser hardware at ~30 FPS"""
        logger.info("DAC output thread started")
        frame_time = 1.0 / 30.0  # 30 FPS target
        
        while self.dac_running:
            try:
                start = time.time()
                
                # Generate current frame
                current_time = time.time() - self.start_time
                points = self.shape_generator.generate_shape(self.state, current_time)
                
                # Send to DAC if not in blackout
                if not self.state.blackout and points:
                    self.dac_manager.send_frame(points)
                
                # Maintain frame rate
                elapsed = time.time() - start
                sleep_time = max(0, frame_time - elapsed)
                if sleep_time > 0:
                    time.sleep(sleep_time)
            except Exception as e:
                logger.error(f"Error in DAC output loop: {e}")
                time.sleep(0.1)
        
        logger.info("DAC output thread stopped")
    
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
        
        # Start DAC output thread if enabled
        if self.enable_dac:
            self.dac_running = True
            self.dac_thread = threading.Thread(target=self._dac_output_loop, daemon=True)
            self.dac_thread.start()
            logger.info("EtherDream DAC output thread started")
        
        # Start Flask app
        logger.info(f"Starting web interface on http://0.0.0.0:{self.http_port}")
        logger.info("=" * 60)
        logger.info("BeamCommander is ready!")
        logger.info(f"  Web Interface: http://localhost:{self.http_port}")
        logger.info(f"  OSC Port: {self.osc_port}")
        if self.enable_dac:
            logger.info(f"  DAC Output: ENABLED")
        logger.info("=" * 60)
        
        # Run Flask in main thread
        self.app.run(host='0.0.0.0', port=self.http_port, debug=False, threaded=True)
    
    def stop(self):
        """Stop the BeamCommander server"""
        if not self.running:
            return
        
        logger.info("Stopping BeamCommander server...")
        self.running = False
        
        # Stop DAC output thread
        if self.dac_running:
            self.dac_running = False
            if self.dac_thread:
                self.dac_thread.join(timeout=2.0)
        
        # Stop DAC manager
        if self.dac_manager:
            self.dac_manager.stop()
        
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
    parser.add_argument('--enable-dac', action='store_true',
                       help='Enable EtherDream DAC output')
    parser.add_argument('--dac-ip', type=str, default=None,
                       help='EtherDream DAC IP address (default: auto-discover)')
    
    args = parser.parse_args()
    
    # Set logging level
    logging.getLogger().setLevel(getattr(logging, args.log_level))
    
    # Create and start server
    server = BeamCommanderServer(
        osc_port=args.osc_port,
        http_port=args.http_port,
        enable_dac=args.enable_dac,
        dac_ip=args.dac_ip
    )
    
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
