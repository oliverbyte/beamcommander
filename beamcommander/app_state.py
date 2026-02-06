"""
Application state management for BeamCommander
"""
from enum import Enum
from dataclasses import dataclass, asdict
from typing import Tuple, Dict, Any
import colorsys
import threading


class Shape(Enum):
    """Laser shape types"""
    CIRCLE = "circle"
    LINE = "line"
    TRIANGLE = "triangle"
    SQUARE = "square"
    WAVE = "wave"
    STATIC_WAVE = "staticwave"


class ColorSel(Enum):
    """Predefined color selections"""
    BLUE = "blue"
    RED = "red"
    GREEN = "green"


class Movement(Enum):
    """Movement pattern types"""
    NONE = "none"
    CIRCLE = "circle"
    PAN = "pan"
    TILT = "tilt"
    EIGHT = "eight"
    RANDOM = "random"


class BeamFx(Enum):
    """Beam effect types"""
    NONE = "none"
    PRISMA = "prisma"


@dataclass
class AppState:
    """
    Central application state for laser control
    Thread-safe state management for real-time control
    """
    # Shape and color
    current_shape: Shape = Shape.CIRCLE
    current_color: ColorSel = ColorSel.BLUE
    movement: Movement = Movement.NONE
    
    # Custom color (when enabled, overrides current_color)
    use_custom_color: bool = False
    custom_r: float = 0.0
    custom_g: float = 0.2
    custom_b: float = 1.0
    
    # Wave parameters
    wave_frequency: float = 1.0  # cycles across width
    wave_amplitude: float = 0.45  # fraction of half-height
    wave_speed: float = 0.0  # cycles per second
    
    # Rainbow effects
    rainbow_speed: float = 0.0  # hue cycles per second
    rainbow_amount: float = 0.0  # blend amount [0..1]
    rainbow_blend: float = 1.0  # smooth gradient [0..1]
    
    # Movement controls
    move_speed: float = 0.30  # cycles per second
    move_size: float = 0.50  # amplitude [0..1]
    
    # Rotation
    rotation_speed: float = 0.0  # rotations per second
    
    # Shape scale and position
    shape_scale: float = 0.0  # [-1..1]
    pos_norm_x: float = 0.0  # [-1..1]
    pos_norm_y: float = 0.0  # [-1..1]
    
    # Axis controls
    invert_x: bool = False
    blackout: bool = False
    
    # Effects
    beam_fx: BeamFx = BeamFx.NONE
    master_brightness: float = 1.0  # [0..1]
    dot_amount: float = 1.0  # [0..1], 0=invisible, 1=solid
    
    # Flicker/strobe
    flicker_hz: float = 0.0  # flicker frequency in Hz
    
    def __post_init__(self):
        """Initialize thread lock for safe concurrent access"""
        self._lock = threading.RLock()
    
    def to_color_rgb(self, rainbow_hue_01: float = -1.0) -> Tuple[int, int, int]:
        """
        Convert current color state to RGB values (0-255)
        
        Args:
            rainbow_hue_01: Optional rainbow hue [0..1], -1 to disable
            
        Returns:
            Tuple of (r, g, b) values in range [0, 255]
        """
        with self._lock:
            if self.use_custom_color:
                r = max(0.0, min(1.0, self.custom_r))
                g = max(0.0, min(1.0, self.custom_g))
                b = max(0.0, min(1.0, self.custom_b))
                base_color = (int(r * 255), int(g * 255), int(b * 255))
            else:
                # Predefined colors
                color_map = {
                    ColorSel.RED: (255, 0, 20),
                    ColorSel.GREEN: (0, 220, 80),
                    ColorSel.BLUE: (0, 50, 255),
                }
                base_color = color_map.get(self.current_color, (0, 50, 255))
            
            # Apply rainbow blend if requested
            if rainbow_hue_01 >= 0.0 and self.rainbow_amount > 0.0:
                # Convert base color to 0-1 range
                br, bg, bb = base_color[0] / 255.0, base_color[1] / 255.0, base_color[2] / 255.0
                
                # Get rainbow color from hue
                rainbow_r, rainbow_g, rainbow_b = colorsys.hsv_to_rgb(rainbow_hue_01, 1.0, 1.0)
                
                # Blend
                amt = max(0.0, min(1.0, self.rainbow_amount))
                r = br * (1 - amt) + rainbow_r * amt
                g = bg * (1 - amt) + rainbow_g * amt
                b = bb * (1 - amt) + rainbow_b * amt
                
                return (int(r * 255), int(g * 255), int(b * 255))
            
            return base_color
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert state to dictionary for serialization"""
        with self._lock:
            return {
                'current_shape': self.current_shape.value,
                'current_color': self.current_color.value,
                'movement': self.movement.value,
                'use_custom_color': self.use_custom_color,
                'custom_r': self.custom_r,
                'custom_g': self.custom_g,
                'custom_b': self.custom_b,
                'wave_frequency': self.wave_frequency,
                'wave_amplitude': self.wave_amplitude,
                'wave_speed': self.wave_speed,
                'rainbow_speed': self.rainbow_speed,
                'rainbow_amount': self.rainbow_amount,
                'rainbow_blend': self.rainbow_blend,
                'move_speed': self.move_speed,
                'move_size': self.move_size,
                'rotation_speed': self.rotation_speed,
                'shape_scale': self.shape_scale,
                'pos_norm_x': self.pos_norm_x,
                'pos_norm_y': self.pos_norm_y,
                'invert_x': self.invert_x,
                'blackout': self.blackout,
                'beam_fx': self.beam_fx.value,
                'master_brightness': self.master_brightness,
                'dot_amount': self.dot_amount,
                'flicker_hz': self.flicker_hz,
            }


@dataclass
class CueState:
    """Snapshot of application state for cue recall"""
    shape: str = "circle"
    color_sel: str = "blue"
    movement: str = "none"
    beam_fx: str = "none"
    use_custom: bool = False
    r: float = 0.0
    g: float = 0.2
    b: float = 1.0
    rainbow_speed: float = 0.0
    rainbow_amount: float = 0.0
    rainbow_blend: float = 1.0
    wave_frequency: float = 1.0
    wave_amplitude: float = 0.45
    wave_speed: float = 0.0
    move_speed: float = 0.30
    move_size: float = 0.50
    rotation_speed: float = 0.0
    shape_scale: float = 0.0
    pos_x: float = 0.0
    pos_y: float = 0.0
    dot_amount: float = 1.0
    flicker_hz: float = 0.0
    populated: bool = False
    
    @classmethod
    def from_app_state(cls, state: AppState) -> 'CueState':
        """Create a cue snapshot from current app state"""
        return cls(
            shape=state.current_shape.value,
            color_sel=state.current_color.value,
            movement=state.movement.value,
            beam_fx=state.beam_fx.value,
            use_custom=state.use_custom_color,
            r=state.custom_r,
            g=state.custom_g,
            b=state.custom_b,
            rainbow_speed=state.rainbow_speed,
            rainbow_amount=state.rainbow_amount,
            rainbow_blend=state.rainbow_blend,
            wave_frequency=state.wave_frequency,
            wave_amplitude=state.wave_amplitude,
            wave_speed=state.wave_speed,
            move_speed=state.move_speed,
            move_size=state.move_size,
            rotation_speed=state.rotation_speed,
            shape_scale=state.shape_scale,
            pos_x=state.pos_norm_x,
            pos_y=state.pos_norm_y,
            dot_amount=state.dot_amount,
            flicker_hz=state.flicker_hz,
            populated=True
        )
    
    def apply_to_app_state(self, state: AppState) -> None:
        """Apply this cue to the application state"""
        if not self.populated:
            return
            
        state.current_shape = Shape(self.shape)
        state.current_color = ColorSel(self.color_sel)
        state.movement = Movement(self.movement)
        state.beam_fx = BeamFx(self.beam_fx)
        state.use_custom_color = self.use_custom
        state.custom_r = self.r
        state.custom_g = self.g
        state.custom_b = self.b
        state.rainbow_speed = self.rainbow_speed
        state.rainbow_amount = self.rainbow_amount
        state.rainbow_blend = self.rainbow_blend
        state.wave_frequency = self.wave_frequency
        state.wave_amplitude = self.wave_amplitude
        state.wave_speed = self.wave_speed
        state.move_speed = self.move_speed
        state.move_size = self.move_size
        state.rotation_speed = self.rotation_speed
        state.shape_scale = self.shape_scale
        state.pos_norm_x = self.pos_x
        state.pos_norm_y = self.pos_y
        state.dot_amount = self.dot_amount
        state.flicker_hz = self.flicker_hz
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        return asdict(self)
