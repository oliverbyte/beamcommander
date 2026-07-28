"""
OSC message receiver and handler for BeamCommander
"""
import logging
from pythonosc import dispatcher, osc_server
from pythonosc.udp_client import SimpleUDPClient
import threading
from typing import Optional, Callable, List, Any
from .app_state import AppState, Shape, ColorSel, Movement, BeamFx

logger = logging.getLogger(__name__)


class OSCReceiver:
    """
    OSC receiver that handles incoming OSC messages and updates application state
    """
    
    def __init__(self, state: AppState, port: int = 9000):
        """
        Initialize OSC receiver
        
        Args:
            state: Application state to update
            port: UDP port to listen on (default: 9000)
        """
        self.state = state
        self.port = port
        self.server: Optional[osc_server.ThreadingOSCUDPServer] = None
        self.server_thread: Optional[threading.Thread] = None
        self.running = False
        
        # Cue management
        self.save_armed = False
        self.flash_active = False
        self.flash_prev_brightness = 0.0
        self.flash_release_ms = 150
        
        # Callback for cue save/recall
        self.on_cue_save: Optional[Callable[[int], None]] = None
        self.on_cue_recall: Optional[Callable[[int], None]] = None
        
    def setup_dispatcher(self) -> dispatcher.Dispatcher:
        """Create and configure the OSC dispatcher with all message handlers"""
        disp = dispatcher.Dispatcher()
        
        # Shape generation
        disp.map("/laser/shape", self._handle_shape)
        
        # Color control
        disp.map("/laser/color", self._handle_color)
        
        # Brightness & visual effects
        disp.map("/laser/brightness", self._handle_brightness)
        disp.map("/laser/master/brightness", self._handle_brightness)
        disp.map("/laser/dotted", self._handle_dotted)
        disp.map("/laser/flicker", self._handle_flicker)
        disp.map("/laser/scanrate", self._handle_flicker)  # Alias
        
        # Positioning & scaling
        disp.map("/laser/position", self._handle_position)
        disp.map("/laser/position/x", self._handle_position_x)
        disp.map("/laser/position/y", self._handle_position_y)
        disp.map("/laser/shape/scale", self._handle_scale)
        disp.map("/laser/rotation/speed", self._handle_rotation_speed)
        
        # Wave pattern controls
        disp.map("/laser/wave/frequency", self._handle_wave_frequency)
        disp.map("/laser/wave/amplitude", self._handle_wave_amplitude)
        disp.map("/laser/wave/speed", self._handle_wave_speed)
        
        # Rainbow effects
        disp.map("/laser/rainbow/amount", self._handle_rainbow_amount)
        disp.map("/laser/rainbow/speed", self._handle_rainbow_speed)
        disp.map("/laser/rainbow/blend", self._handle_rainbow_blend)
        
        # Movement patterns
        disp.map("/move/mode", self._handle_move_mode)
        disp.map("/move/size", self._handle_move_size)
        disp.map("/move/speed", self._handle_move_speed)
        
        # Flash controls
        disp.map("/flash", self._handle_flash)
        disp.map("/flash/release_ms", self._handle_flash_release_ms)
        
        # Cue system
        disp.map("/cue/save", self._handle_cue_save)
        disp.map("/cue/*", self._handle_cue)
        
        # Blackout
        disp.map("/blackout", self._handle_blackout)
        
        logger.info("OSC dispatcher configured with all message handlers")
        return disp
    
    def start(self):
        """Start the OSC server in a separate thread"""
        if self.running:
            logger.warning("OSC server already running")
            return
        
        disp = self.setup_dispatcher()
        self.server = osc_server.ThreadingOSCUDPServer(
            ("0.0.0.0", self.port), disp
        )
        
        self.server_thread = threading.Thread(
            target=self.server.serve_forever,
            daemon=True
        )
        self.running = True
        self.server_thread.start()
        logger.info(f"OSC server started on port {self.port}")
    
    def stop(self):
        """Stop the OSC server"""
        if not self.running:
            return
        
        self.running = False
        if self.server:
            self.server.shutdown()
            self.server = None
        
        if self.server_thread:
            self.server_thread.join(timeout=2.0)
            self.server_thread = None
        
        logger.info("OSC server stopped")
    
    # Handler methods
    def _handle_shape(self, address: str, *args: Any):
        """Handle /laser/shape message"""
        if not args:
            return
        shape_str = str(args[0]).lower()
        try:
            self.state.current_shape = Shape(shape_str)
            logger.debug(f"Shape set to: {shape_str}")
        except ValueError:
            logger.warning(f"Invalid shape: {shape_str}")
    
    def _handle_color(self, address: str, *args: Any):
        """Handle /laser/color message"""
        if not args:
            return
        
        # Check if it's a named color or RGB values
        if len(args) == 1 and isinstance(args[0], str):
            # Named color
            color_str = args[0].lower()
            try:
                self.state.current_color = ColorSel(color_str)
                self.state.use_custom_color = False
                logger.debug(f"Color set to: {color_str}")
            except ValueError:
                logger.warning(f"Invalid color: {color_str}")
        elif len(args) >= 3:
            # RGB values
            r, g, b = float(args[0]), float(args[1]), float(args[2])
            # Normalize if values are > 1 (assume 0-255 range)
            if r > 1.0 or g > 1.0 or b > 1.0:
                r, g, b = r / 255.0, g / 255.0, b / 255.0
            self.state.custom_r = max(0.0, min(1.0, r))
            self.state.custom_g = max(0.0, min(1.0, g))
            self.state.custom_b = max(0.0, min(1.0, b))
            self.state.use_custom_color = True
            logger.debug(f"Custom color set to RGB: ({r:.2f}, {g:.2f}, {b:.2f})")
    
    def _handle_brightness(self, address: str, *args: Any):
        """Handle /laser/brightness or /laser/master/brightness"""
        if not args:
            return
        value = float(args[0])
        # Normalize if value is > 1 (assume 0-255 range)
        if value > 1.0:
            value = value / 255.0
        self.state.master_brightness = max(0.0, min(1.0, value))
        logger.debug(f"Brightness set to: {self.state.master_brightness:.2f}")
    
    def _handle_dotted(self, address: str, *args: Any):
        """Handle /laser/dotted message"""
        if not args:
            return
        value = float(args[0])
        if value > 1.0:
            value = value / 255.0
        self.state.dot_amount = max(0.0, min(1.0, value))
        logger.debug(f"Dotted amount set to: {self.state.dot_amount:.2f}")
    
    def _handle_flicker(self, address: str, *args: Any):
        """Handle /laser/flicker or /laser/scanrate message"""
        if not args:
            return
        hz = float(args[0])
        self.state.flicker_hz = max(0.0, hz)
        logger.debug(f"Flicker rate set to: {hz} Hz")
    
    def _handle_position(self, address: str, *args: Any):
        """Handle /laser/position x y message"""
        if len(args) < 2:
            return
        x, y = float(args[0]), float(args[1])
        self.state.pos_norm_x = max(-1.0, min(1.0, x))
        self.state.pos_norm_y = max(-1.0, min(1.0, y))
        logger.debug(f"Position set to: ({x:.2f}, {y:.2f})")
    
    def _handle_position_x(self, address: str, *args: Any):
        """Handle /laser/position/x message"""
        if not args:
            return
        x = float(args[0])
        self.state.pos_norm_x = max(-1.0, min(1.0, x))
    
    def _handle_position_y(self, address: str, *args: Any):
        """Handle /laser/position/y message"""
        if not args:
            return
        y = float(args[0])
        self.state.pos_norm_y = max(-1.0, min(1.0, y))
    
    def _handle_scale(self, address: str, *args: Any):
        """Handle /laser/shape/scale message"""
        if not args:
            return
        scale = float(args[0])
        self.state.shape_scale = max(-1.0, min(1.0, scale))
        logger.debug(f"Shape scale set to: {scale:.2f}")
    
    def _handle_rotation_speed(self, address: str, *args: Any):
        """Handle /laser/rotation/speed message"""
        if not args:
            return
        speed = float(args[0])
        self.state.rotation_speed = speed
        logger.debug(f"Rotation speed set to: {speed:.2f} rot/sec")
    
    def _handle_wave_frequency(self, address: str, *args: Any):
        """Handle /laser/wave/frequency message"""
        if not args:
            return
        freq = float(args[0])
        self.state.wave_frequency = max(0.1, freq)
    
    def _handle_wave_amplitude(self, address: str, *args: Any):
        """Handle /laser/wave/amplitude message"""
        if not args:
            return
        amp = float(args[0])
        self.state.wave_amplitude = max(0.0, min(1.0, amp))
    
    def _handle_wave_speed(self, address: str, *args: Any):
        """Handle /laser/wave/speed message"""
        if not args:
            return
        speed = float(args[0])
        self.state.wave_speed = speed
    
    def _handle_rainbow_amount(self, address: str, *args: Any):
        """Handle /laser/rainbow/amount message"""
        if not args:
            return
        amount = float(args[0])
        self.state.rainbow_amount = max(0.0, min(1.0, amount))
    
    def _handle_rainbow_speed(self, address: str, *args: Any):
        """Handle /laser/rainbow/speed message"""
        if not args:
            return
        speed = float(args[0])
        self.state.rainbow_speed = speed
    
    def _handle_rainbow_blend(self, address: str, *args: Any):
        """Handle /laser/rainbow/blend message"""
        if not args:
            return
        blend = float(args[0])
        self.state.rainbow_blend = max(0.0, min(1.0, blend))
    
    def _handle_move_mode(self, address: str, *args: Any):
        """Handle /move/mode message"""
        if not args:
            return
        mode_str = str(args[0]).lower()
        # Handle aliases
        if mode_str in ["off", "none"]:
            mode_str = "none"
        elif mode_str in ["eight", "figure8", "8"]:
            mode_str = "eight"
        
        try:
            self.state.movement = Movement(mode_str)
            logger.debug(f"Movement mode set to: {mode_str}")
        except ValueError:
            logger.warning(f"Invalid movement mode: {mode_str}")
    
    def _handle_move_size(self, address: str, *args: Any):
        """Handle /move/size message"""
        if not args:
            return
        size = float(args[0])
        if size > 1.0:
            size = size / 255.0
        self.state.move_size = max(0.0, min(1.0, size))
    
    def _handle_move_speed(self, address: str, *args: Any):
        """Handle /move/speed message"""
        if not args:
            return
        speed = float(args[0])
        self.state.move_speed = speed
    
    def _handle_flash(self, address: str, *args: Any):
        """Handle /flash message"""
        if not args:
            return
        value = int(args[0])
        if value == 1:
            # Flash ON - save current brightness and set to max
            self.flash_prev_brightness = self.state.master_brightness
            self.state.master_brightness = 1.0
            self.flash_active = True
            logger.debug("Flash activated")
        else:
            # Flash OFF - restore previous brightness
            if self.flash_active:
                self.state.master_brightness = self.flash_prev_brightness
                self.flash_active = False
                logger.debug("Flash released")
    
    def _handle_flash_release_ms(self, address: str, *args: Any):
        """Handle /flash/release_ms message"""
        if not args:
            return
        ms = int(args[0])
        self.flash_release_ms = max(0, min(60000, ms))
    
    def _handle_cue_save(self, address: str, *args: Any):
        """Handle /cue/save message to arm save mode"""
        self.save_armed = True
        logger.info("Cue save mode armed")
    
    def _handle_cue(self, address: str, *args: Any):
        """Handle /cue/N message for save or recall"""
        # Extract cue number from address (e.g., /cue/5 -> 5)
        try:
            parts = address.split('/')
            if len(parts) < 3:
                return
            cue_num = int(parts[2])
            
            if self.save_armed:
                # Save current state to cue
                if self.on_cue_save:
                    self.on_cue_save(cue_num)
                self.save_armed = False
                logger.info(f"Saved cue {cue_num}")
            else:
                # Recall cue
                if self.on_cue_recall:
                    self.on_cue_recall(cue_num)
                logger.info(f"Recalled cue {cue_num}")
        except (ValueError, IndexError) as e:
            logger.warning(f"Invalid cue address: {address}")
    
    def _handle_blackout(self, address: str, *args: Any):
        """Handle /blackout message"""
        if not args:
            return
        value = int(args[0])
        self.state.blackout = (value != 0)
        logger.debug(f"Blackout: {self.state.blackout}")
