"""
Shape generation for laser output
"""
import math
from typing import List, Tuple
from .app_state import Shape, AppState


class ShapeGenerator:
    """
    Generates point data for various laser shapes
    """
    
    def __init__(self, width: int = 800, height: int = 600):
        """
        Initialize shape generator
        
        Args:
            width: Canvas width in points
            height: Canvas height in points
        """
        self.width = width
        self.height = height
        self.center_x = width / 2
        self.center_y = height / 2
    
    def generate_shape(self, state: AppState, time_sec: float) -> List[Tuple[float, float, int, int, int]]:
        """
        Generate point list for current shape with position, color
        
        Args:
            state: Current application state
            time_sec: Current time in seconds for animations
            
        Returns:
            List of tuples (x, y, r, g, b) representing points
        """
        # Calculate scale factor from normalized scale
        # Map [-1..1] to a geometric scale factor
        scale_norm = state.shape_scale
        if scale_norm >= 0:
            scale_factor = 1.0 + scale_norm * 2.0  # [1.0 .. 3.0]
        else:
            scale_factor = 1.0 + scale_norm * 0.7  # [0.3 .. 1.0]
        
        # Calculate rotation angle
        rotation_angle = state.rotation_speed * time_sec * 2 * math.pi
        
        # Calculate movement offset
        move_offset_x, move_offset_y = self._calculate_movement(state, time_sec)
        
        # Generate base shape points
        if state.current_shape == Shape.CIRCLE:
            points = self._generate_circle(scale_factor)
        elif state.current_shape == Shape.LINE:
            points = self._generate_line(scale_factor)
        elif state.current_shape == Shape.TRIANGLE:
            points = self._generate_triangle(scale_factor)
        elif state.current_shape == Shape.SQUARE:
            points = self._generate_square(scale_factor)
        elif state.current_shape == Shape.WAVE:
            points = self._generate_wave(state, time_sec, scale_factor)
        elif state.current_shape == Shape.STATIC_WAVE:
            points = self._generate_static_wave(state, time_sec, scale_factor)
        else:
            points = self._generate_circle(scale_factor)
        
        # Apply rotation
        if rotation_angle != 0:
            points = self._rotate_points(points, rotation_angle)
        
        # Apply position offset (manual + movement)
        pos_x = state.pos_norm_x * self.width * 0.5 + move_offset_x
        pos_y = state.pos_norm_y * self.height * 0.5 + move_offset_y
        
        # Apply axis inversion
        if state.invert_x:
            pos_x = -pos_x
        
        # Calculate rainbow hue for color animation
        rainbow_phase = (state.rainbow_speed * time_sec) % 1.0
        
        # Generate colored points
        colored_points = []
        num_points = len(points)
        for i, (x, y) in enumerate(points):
            # Calculate position-dependent rainbow hue
            hue = (rainbow_phase + i / num_points * state.rainbow_amount) % 1.0 if state.rainbow_amount > 0 else -1.0
            
            # Get color
            r, g, b = state.to_color_rgb(hue)
            
            # Apply master brightness
            r = int(r * state.master_brightness)
            g = int(g * state.master_brightness)
            b = int(b * state.master_brightness)
            
            # Translate to final position
            final_x = x + self.center_x + pos_x
            final_y = y + self.center_y + pos_y
            
            colored_points.append((final_x, final_y, r, g, b))
        
        # Apply dot amount (reduce points for dotted effect)
        if state.dot_amount < 1.0:
            # Keep only a fraction of points
            keep_ratio = max(0.01, state.dot_amount)
            step = int(1.0 / keep_ratio)
            colored_points = colored_points[::step]
        
        return colored_points
    
    def _calculate_movement(self, state: AppState, time_sec: float) -> Tuple[float, float]:
        """Calculate movement offset based on movement mode"""
        if state.movement.value == "none":
            return (0.0, 0.0)
        
        phase = state.move_speed * time_sec * 2 * math.pi
        amplitude = state.move_size * min(self.width, self.height) * 0.3
        
        if state.movement.value == "circle":
            x = amplitude * math.cos(phase)
            y = amplitude * math.sin(phase)
        elif state.movement.value == "pan":
            x = amplitude * math.sin(phase)
            y = 0.0
        elif state.movement.value == "tilt":
            x = 0.0
            y = amplitude * math.sin(phase)
        elif state.movement.value == "eight":
            # Figure-8 pattern (Lissajous curve)
            x = amplitude * math.sin(phase)
            y = amplitude * math.sin(2 * phase)
        elif state.movement.value == "random":
            # Simple pseudo-random using sine waves with different frequencies
            x = amplitude * (math.sin(phase * 1.3) + math.sin(phase * 2.7)) / 2
            y = amplitude * (math.sin(phase * 1.7) + math.sin(phase * 3.1)) / 2
        else:
            x, y = 0.0, 0.0
        
        return (x, y)
    
    def _rotate_points(self, points: List[Tuple[float, float]], angle: float) -> List[Tuple[float, float]]:
        """Rotate points around origin"""
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)
        rotated = []
        for x, y in points:
            new_x = x * cos_a - y * sin_a
            new_y = x * sin_a + y * cos_a
            rotated.append((new_x, new_y))
        return rotated
    
    def _generate_circle(self, scale: float, num_points: int = 100) -> List[Tuple[float, float]]:
        """Generate circle points"""
        radius = min(self.width, self.height) * 0.3 * scale
        points = []
        for i in range(num_points):
            angle = (i / num_points) * 2 * math.pi
            x = radius * math.cos(angle)
            y = radius * math.sin(angle)
            points.append((x, y))
        return points
    
    def _generate_line(self, scale: float, num_points: int = 50) -> List[Tuple[float, float]]:
        """Generate line points"""
        length = min(self.width, self.height) * 0.6 * scale
        points = []
        for i in range(num_points):
            t = (i / (num_points - 1)) - 0.5  # [-0.5 to 0.5]
            x = t * length
            y = 0
            points.append((x, y))
        return points
    
    def _generate_triangle(self, scale: float, num_points: int = 75) -> List[Tuple[float, float]]:
        """Generate triangle points"""
        size = min(self.width, self.height) * 0.3 * scale
        points = []
        # Three vertices of equilateral triangle
        vertices = [
            (0, -size),
            (-size * 0.866, size * 0.5),
            (size * 0.866, size * 0.5),
        ]
        # Draw lines between vertices
        points_per_edge = num_points // 3
        for i in range(3):
            v1 = vertices[i]
            v2 = vertices[(i + 1) % 3]
            for j in range(points_per_edge):
                t = j / points_per_edge
                x = v1[0] * (1 - t) + v2[0] * t
                y = v1[1] * (1 - t) + v2[1] * t
                points.append((x, y))
        return points
    
    def _generate_square(self, scale: float, num_points: int = 80) -> List[Tuple[float, float]]:
        """Generate square points"""
        size = min(self.width, self.height) * 0.3 * scale
        points = []
        # Four vertices
        vertices = [
            (-size, -size),
            (size, -size),
            (size, size),
            (-size, size),
        ]
        # Draw lines between vertices
        points_per_edge = num_points // 4
        for i in range(4):
            v1 = vertices[i]
            v2 = vertices[(i + 1) % 4]
            for j in range(points_per_edge):
                t = j / points_per_edge
                x = v1[0] * (1 - t) + v2[0] * t
                y = v1[1] * (1 - t) + v2[1] * t
                points.append((x, y))
        return points
    
    def _generate_wave(self, state: AppState, time_sec: float, scale: float, num_points: int = 100) -> List[Tuple[float, float]]:
        """Generate animated wave points"""
        width = min(self.width, self.height) * 0.6 * scale
        amplitude = width * state.wave_amplitude
        phase = state.wave_speed * time_sec * 2 * math.pi
        
        points = []
        for i in range(num_points):
            t = (i / (num_points - 1)) - 0.5  # [-0.5 to 0.5]
            x = t * width
            y = amplitude * math.sin(state.wave_frequency * 2 * math.pi * t + phase)
            points.append((x, y))
        return points
    
    def _generate_static_wave(self, state: AppState, time_sec: float, scale: float, num_points: int = 100) -> List[Tuple[float, float]]:
        """Generate static wave points (phase doesn't animate)"""
        width = min(self.width, self.height) * 0.6 * scale
        amplitude = width * state.wave_amplitude
        
        points = []
        for i in range(num_points):
            t = (i / (num_points - 1)) - 0.5  # [-0.5 to 0.5]
            x = t * width
            y = amplitude * math.sin(state.wave_frequency * 2 * math.pi * t)
            points.append((x, y))
        return points
