# BeamCommander - Python Edition 🎆

**Generic, Cross-Platform Laser Control System**

[![Python](https://img.shields.io/badge/python-3.8+-blue.svg)](https://www.python.org/downloads/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE.md)
[![Platform](https://img.shields.io/badge/platform-linux%20%7C%20macos%20%7C%20windows-lightgrey.svg)](https://github.com/oliverbyte/beamcommander)

BeamCommander 2.0 is a complete rewrite in Python, making it a truly generic and cross-platform laser control system. Control your laser shows in real-time using OSC commands through an intuitive browser-based interface.

## ✨ What's New in 2.0

- **🐍 Pure Python**: No more C++, openFrameworks, or platform-specific dependencies
- **🌐 Browser UI**: Control lasers from any device with a web browser
- **🖥️ Cross-Platform**: Works on Linux, macOS, Windows, and any OS that runs Python
- **📦 Simple Installation**: Just Python 3.8+ and pip
- **🔌 Extensible**: Easy to add new features, shapes, and integrations

## 🚀 Quick Start

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/oliverbyte/beamcommander.git
   cd beamcommander
   ```

2. **Install BeamCommander**
   ```bash
   pip install -e .
   ```
   
   Or manually:
   ```bash
   pip install -r requirements.txt
   ```

3. **Run the server**
   ```bash
   ./start.sh
   ```
   
   Or directly:
   ```bash
   python3 -m beamcommander.server
   ```

4. **Open the web interface**
   - Navigate to http://localhost:8080 in your browser
   - Control your laser show from the web UI!

### System Requirements

- **Python**: 3.8 or higher
- **Operating System**: Linux, macOS, Windows, or any Python-compatible OS
- **Browser**: Any modern browser (Chrome, Firefox, Safari, Edge)
- **Network**: For OSC control and web interface

## 🎮 Usage

### Web Interface

The browser-based UI provides intuitive controls for:

- **Shape Selection**: Circle, Line, Triangle, Square, Wave patterns
- **Color Control**: Predefined colors (Red, Green, Blue) or custom RGB
- **Movement Patterns**: Circle, Pan, Tilt, Figure-8, Random
- **Rainbow Effects**: Speed and intensity control
- **Transform Controls**: Position, Scale, Rotation
- **Visual Effects**: Brightness, Dot patterns, Blackout

### OSC Control

BeamCommander listens for OSC messages on **UDP port 9000**. Send commands from any OSC-compatible software:

#### Basic Commands

```bash
# Set shape
/laser/shape circle|line|triangle|square|wave|staticwave

# Set color
/laser/color blue|red|green
/laser/color <r> <g> <b>  # RGB values 0-1 or 0-255

# Brightness and effects
/laser/brightness <0-1>
/laser/dotted <0-1>
/laser/flicker <hz>

# Position and scale
/laser/position <x> <y>  # Both -1 to +1
/laser/shape/scale <-1 to +1>
/laser/rotation/speed <rot/sec>

# Movement
/move/mode none|circle|pan|tilt|eight|random
/move/size <0-1>
/move/speed <cycles/sec>

# Rainbow effects
/laser/rainbow/amount <0-1>
/laser/rainbow/speed <cycles/sec>

# Cue system
/cue/save          # Arm save mode
/cue/<1-30>        # Save or recall cue

# Control
/blackout <0|1>
/flash <0|1>
```

### Python API

You can also use BeamCommander programmatically:

```python
from beamcommander.server import BeamCommanderServer

# Create server
server = BeamCommanderServer(osc_port=9000, http_port=8080)

# Start server (blocking)
server.start()

# Or access state directly
server.state.master_brightness = 0.8
server.state.current_shape = Shape.CIRCLE
```

## 🏗️ Architecture

BeamCommander 2.0 is built with simplicity and extensibility in mind:

```
beamcommander/
├── __init__.py          # Package initialization
├── app_state.py         # Application state management
├── osc_receiver.py      # OSC message handling
├── shapes.py            # Shape generation algorithms
├── cue_manager.py       # Cue save/recall system
├── server.py            # Main Flask server
├── templates/           # HTML templates
│   └── index.html       # Web UI
└── static/              # JavaScript and CSS
    └── app.js           # Web UI logic
```

### Key Components

- **AppState**: Thread-safe state management for all laser parameters
- **OSCReceiver**: Handles incoming OSC messages and updates state
- **ShapeGenerator**: Generates point data for various laser shapes
- **CueManager**: Manages cue save/recall with disk persistence
- **Flask Server**: Serves web UI and provides REST API

## 🔧 Configuration

### Command-Line Options

```bash
python3 -m beamcommander.server --help

Options:
  --osc-port PORT      OSC receiver port (default: 9000)
  --http-port PORT     HTTP server port (default: 8080)
  --log-level LEVEL    Logging level: DEBUG|INFO|WARNING|ERROR
```

### Environment Variables

```bash
# Set log level
export BEAMCOMMANDER_LOG_LEVEL=DEBUG

# Set ports
export BEAMCOMMANDER_OSC_PORT=9000
export BEAMCOMMANDER_HTTP_PORT=8080
```

## 🎨 Extending BeamCommander

### Adding New Shapes

Edit `beamcommander/shapes.py` and add your shape generation method:

```python
def _generate_my_shape(self, scale: float, num_points: int = 100):
    points = []
    # Generate your shape points
    for i in range(num_points):
        x = ...  # Calculate x coordinate
        y = ...  # Calculate y coordinate
        points.append((x, y))
    return points
```

### Adding New OSC Commands

Edit `beamcommander/osc_receiver.py` and add a handler:

```python
def _handle_my_command(self, address: str, *args: Any):
    """Handle /my/command message"""
    if not args:
        return
    value = float(args[0])
    # Update state based on command
    logger.debug(f"My command: {value}")
```

Then register it in `setup_dispatcher()`:

```python
disp.map("/my/command", self._handle_my_command)
```

## 🔌 Hardware Integration

BeamCommander 2.0 provides an abstraction layer for laser hardware. To connect to actual laser DACs:

1. Create a `laser_output.py` module with your DAC driver
2. Implement the point streaming to your hardware
3. Use the shape generator output to drive your DAC

Example integration:

```python
from beamcommander.shapes import ShapeGenerator
from beamcommander.app_state import AppState

# Initialize
state = AppState()
generator = ShapeGenerator()

# Generate points
points = generator.generate_shape(state, time.time())

# Send to your DAC
for x, y, r, g, b in points:
    your_dac.add_point(x, y, r, g, b)
```

## 🐛 Troubleshooting

### Port Already in Use

If you get "Address already in use" errors:

```bash
# Check what's using the port
lsof -i :9000
lsof -i :8080

# Kill the process or use different ports
python3 -m beamcommander.server --osc-port 9001 --http-port 8081
```

### Web UI Not Loading

1. Check the server is running: `curl http://localhost:8080/api/status`
2. Check firewall settings allow connections to port 8080
3. Try accessing via IP address instead of localhost

### OSC Messages Not Received

1. Verify OSC sender is targeting correct IP and port
2. Check firewall allows UDP port 9000
3. Enable debug logging: `--log-level DEBUG`

## 📚 Documentation

- **OSC API Reference**: See comments in `osc_receiver.py`
- **Shape Generation**: See `shapes.py` for algorithms
- **State Management**: See `app_state.py` for all parameters

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

### Development Setup

```bash
# Clone the repo
git clone https://github.com/oliverbyte/beamcommander.git
cd beamcommander

# Create virtual environment
python3 -m venv venv
source venv/bin/activate

# Install in development mode
pip install -e .

# Run with debug logging
python3 -m beamcommander.server --log-level DEBUG
```

## 📋 Migration from v1.x (C++ Version)

If you're migrating from the old C++/openFrameworks version:

1. **OSC Commands**: All OSC commands remain the same
2. **Cues**: Cue files are not compatible; you'll need to recreate them
3. **MIDI**: MIDI support is planned for a future release
4. **Hardware**: You'll need to implement your DAC driver (see Hardware Integration)

The old C++ code is archived in the `openframeworks-src-master` directory.

## 📄 License

BeamCommander is released under the MIT License. See [LICENSE.md](LICENSE.md) for details.

## 🙏 Acknowledgments

- Original ofxLaser framework by [Seb Lee-Delisle](https://github.com/sebleedelisle)
- OpenFrameworks community for inspiration
- Contributors and users of BeamCommander v1.x

## 📧 Contact

For questions, suggestions, or collaboration:
- **Email**: info@OliverByte.de
- **GitHub**: [oliverbyte/beamcommander](https://github.com/oliverbyte/beamcommander)
- **Website**: [oliverbyte.github.io/beamcommander](https://oliverbyte.github.io/beamcommander/)

---

**Made with ❤️ for the laser art community**
