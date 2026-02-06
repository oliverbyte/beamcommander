# Migration Guide: C++ to Python

## Overview

BeamCommander 2.0 is a complete rewrite from C++/openFrameworks to Python. This guide helps users and developers migrate from the legacy C++ version to the new Python version.

## Key Changes

### ✅ What Stayed the Same

- **OSC Commands**: All OSC command addresses and parameters remain compatible
- **OSC Port**: Still uses UDP port 9000 by default
- **Core Features**: All shapes, colors, movements, and effects are preserved
- **Cue System**: Concept remains the same (save/recall)

### 🔄 What Changed

#### Platform Independence
- **Before**: macOS only, required Xcode and specific openFrameworks version
- **After**: Works on Linux, macOS, Windows, and any Python-compatible OS

#### User Interface
- **Before**: Native desktop application with ImGui interface
- **After**: Browser-based UI accessible from any device on the network

#### Hardware Integration
- **Before**: Built-in support for laser DACs via ofxLaser
- **After**: Abstraction layer provided; requires custom DAC driver implementation

#### MIDI Support
- **Before**: Built-in MIDI controller support (Akai APC40)
- **After**: Planned for future release; currently OSC-only

#### Installation
- **Before**: Complex build process with multiple dependencies
- **After**: Simple `pip install` or `./start.sh`

## Migration Steps

### For End Users

1. **Backup your cue files** (if you want to manually recreate them)
   ```bash
   cp ~/Library/Application\ Support/BeamCommander/cues.json ~/Desktop/cues_backup.json
   ```

2. **Install Python 3.8+**
   - macOS: `brew install python3`
   - Linux: `sudo apt install python3 python3-pip`
   - Windows: Download from python.org

3. **Install BeamCommander 2.0**
   ```bash
   cd beamcommander
   pip install -r requirements.txt
   ```

4. **Start the server**
   ```bash
   ./start.sh
   ```

5. **Access the web UI**
   - Open http://localhost:8080 in your browser
   - Control from any device on your network!

### For Developers

#### Adding Custom DAC Support

Create a `laser_output.py` module:

```python
from beamcommander.shapes import ShapeGenerator
from beamcommander.app_state import AppState
import your_dac_library

class YourDACOutput:
    def __init__(self):
        self.dac = your_dac_library.connect()
        
    def send_points(self, points):
        for x, y, r, g, b in points:
            self.dac.add_point(x, y, r, g, b)
        self.dac.send()

# Integration
state = AppState()
generator = ShapeGenerator()
dac = YourDACOutput()

while True:
    points = generator.generate_shape(state, time.time())
    dac.send_points(points)
```

#### Porting Custom Shapes

**Before (C++):**
```cpp
void ofApp::drawMyShape() {
    ofPolyline shape;
    for (int i = 0; i < 100; i++) {
        float x = i * 10;
        float y = sin(i * 0.1) * 50;
        shape.addVertex(x, y);
    }
    laser.drawPoly(shape);
}
```

**After (Python):**
```python
def _generate_my_shape(self, scale: float, num_points: int = 100):
    points = []
    for i in range(num_points):
        x = i * 10 * scale
        y = math.sin(i * 0.1) * 50 * scale
        points.append((x, y))
    return points
```

#### Porting OSC Handlers

**Before (C++):**
```cpp
if (m.getAddress() == "/laser/mycommand") {
    float value = m.getArgAsFloat(0);
    state->myParameter = value;
}
```

**After (Python):**
```python
def _handle_my_command(self, address: str, *args):
    if not args:
        return
    value = float(args[0])
    self.state.my_parameter = value

# Register in setup_dispatcher():
disp.map("/laser/mycommand", self._handle_my_command)
```

## Feature Parity Matrix

| Feature | C++ Version | Python Version | Notes |
|---------|------------|----------------|-------|
| OSC Control | ✅ | ✅ | Fully compatible |
| Web UI | ❌ | ✅ | New feature |
| Desktop UI | ✅ | ❌ | Use browser instead |
| Shapes (Circle, Line, etc.) | ✅ | ✅ | All preserved |
| Colors & Rainbow | ✅ | ✅ | Fully compatible |
| Movement Patterns | ✅ | ✅ | All preserved |
| Cue System | ✅ | ✅ | Not file-compatible |
| MIDI Support | ✅ | 🚧 | Planned |
| Laser DAC Support | ✅ | 🔧 | Requires custom driver |
| Cross-platform | ❌ | ✅ | Linux/Mac/Windows |
| Easy Installation | ❌ | ✅ | Single command |

Legend: ✅ Available | ❌ Not Available | 🚧 In Progress | 🔧 Custom Integration Required

## Troubleshooting

### Issue: "My MIDI controller doesn't work"

**Solution**: MIDI support is not yet available in Python version. Options:
1. Wait for future MIDI support release
2. Use OSC-based control instead
3. Create MIDI-to-OSC bridge using external tools

### Issue: "No laser output to my DAC"

**Solution**: Python version provides abstraction layer only. You need to:
1. Identify your DAC model
2. Find/write Python driver for your DAC
3. Integrate with shape generator output

### Issue: "Cue files don't load"

**Solution**: Cue file formats are incompatible. Manually recreate cues:
1. Open old version, load cue
2. Note all parameters
3. Open new version, set parameters
4. Save as new cue

### Issue: "Performance is slower than C++ version"

**Solution**: 
- Python is interpreted, expect some overhead
- For production use, profile and optimize critical paths
- Consider Cython or PyPy for performance-critical code
- The web UI targets 20 FPS, which is sufficient for control

## Benefits of Migration

### For Users
- ✅ Access UI from phone, tablet, or computer
- ✅ No compilation required
- ✅ Works on any operating system
- ✅ Easier to install and update
- ✅ More accessible for non-technical users

### For Developers
- ✅ Python is easier to learn and modify
- ✅ Rich ecosystem of libraries
- ✅ Faster development iteration
- ✅ Better documentation tools
- ✅ Active community support

## Getting Help

- **Documentation**: See README.md for full documentation
- **Issues**: https://github.com/oliverbyte/beamcommander/issues
- **Email**: info@OliverByte.de
- **Legacy Version**: C++ code preserved in `openframeworks-src-master/`

## Roadmap

Future features planned for Python version:

- 🚧 MIDI controller support (Akai APC40, etc.)
- 🚧 Built-in EtherDream DAC support
- 🚧 WebSocket-based OSC bridge for browser control
- 🚧 Advanced preset management
- 🚧 Timeline/sequence editor
- 🚧 DMX integration
- 🚧 Audio reactivity

## Contributing

The Python version is designed to be more accessible for contributions. If you'd like to help:

1. Check open issues
2. Fork the repository
3. Make your changes
4. Submit a pull request

We especially welcome:
- DAC driver implementations
- UI improvements
- Documentation
- Bug reports
- Feature suggestions

---

**Note**: The legacy C++ version remains available in the repository for reference and for users who need specific features not yet available in Python version.
