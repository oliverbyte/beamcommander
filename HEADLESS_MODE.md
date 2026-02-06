# Headless Mode for BeamCommander

## Overview

BeamCommander now supports running in headless mode, allowing the application to run without a graphical user interface. This is perfect for:

- Server deployments
- Remote-controlled installations
- Automated laser shows
- Systems without display hardware
- Docker containers or cloud deployments

## Features

### Console-Only Operation
- No window or GUI required
- All output to console/logs
- Uses existing configuration files from `bin/data/`
- Full OSC command support on port 9000
- MIDI controller support (auto-detected)

### Web-Based 3D Preview
- HTTP server on port 8080
- Real-time laser visualization in browser
- Canvas-based rendering (no WebGL dependencies)
- Shows current shape, color, position, rotation, and movement
- JSON API for laser state at `/api/laser`

## Usage

### Starting Headless Mode

```bash
# Using the helper script
./start_headless.sh

# Or directly with the binary
./bin/BeamCommander.app/Contents/MacOS/BeamCommander --headless
```

### Command-Line Arguments

```
BeamCommander [OPTIONS]

Options:
  --headless        Run in headless mode without UI
  --help, -h        Show help message
```

### Accessing the Web Preview

Once running in headless mode:

1. Open a web browser
2. Navigate to `http://localhost:8080`
3. View real-time laser preview with current parameters

The web interface updates automatically at 20 FPS showing:
- Shape type (line, circle, triangle, square, wave)
- Current color (RGB or named)
- Brightness level
- Position (X, Y coordinates)
- Scale factor
- Rotation angle
- Movement mode and speed

### API Endpoint

**GET /api/laser**

Returns JSON with current laser state:

```json
{
  "shape": "circle",
  "color": "Blue",
  "customColor": {"r": 0.0, "g": 0.0, "b": 1.0},
  "brightness": 0.75,
  "position": {"x": 0.0, "y": 0.0},
  "scale": 1.0,
  "rotation": 0.0,
  "rotationSpeed": 0.5,
  "dotAmount": 1.0,
  "movement": {
    "mode": 1,
    "speed": 1.0,
    "size": 0.5
  },
  "wave": {
    "frequency": 2.0,
    "amplitude": 0.45,
    "speed": 0.0,
    "phase": 0.0
  },
  "rainbow": {
    "speed": 0.0,
    "amount": 0.0,
    "blend": 1.0
  },
  "scanRate": 20000,
  "timestamp": 12345678
}
```

## Configuration

Headless mode uses the same configuration files as normal mode:

- `bin/data/ofxLaser/` - Laser hardware settings
- `bin/data/cues.json` - Saved cue presets
- `bin/data/midi_mapping.json` - MIDI controller mappings

**Important**: Run the application in normal mode first to configure your laser hardware, then switch to headless mode for production use.

## Control Methods

All control methods work identically in headless mode:

### OSC Commands (Port 9000)
```bash
# Example using oscsend
oscsend localhost 9000 /laser/shape s circle
oscsend localhost 9000 /laser/color s blue
oscsend localhost 9000 /laser/brightness f 0.8
```

### MIDI Controllers
- Akai APC40 automatically detected
- All knobs and buttons work as documented
- LED feedback still functions

### Open Stage Control
- Web interface connects to OSC port as usual
- No changes needed to configuration
- Access from any device on network

## Technical Details

### HTTP Server
- Built using ofxTCPServer
- Single-threaded, non-blocking
- Serves static HTML and JSON API
- Port: 8080 (configurable in source)

### Window Management
- Uses OpenFrameworks with GLFW context
- Window created but can be hidden/minimized
- Rendering still occurs for laser output
- No visual feedback required

### Performance
- Minimal overhead for HTTP server
- JSON updates at 20 Hz
- Does not impact laser rendering performance
- Suitable for production use

## Troubleshooting

### Port Already in Use
If port 8080 is already in use, the HTTP server will fail to start. Check console output for errors:
```
Failed to start HTTP server: Address already in use
```

Solution: Stop other services using port 8080 or modify the port in `ofApp.h` and rebuild.

### Web Preview Not Loading
1. Check that BeamCommander is running with `--headless` flag
2. Verify HTTP server started (check console output)
3. Try `http://127.0.0.1:8080` instead of localhost
4. Check firewall settings

### OSC Not Working
OSC functionality is identical to normal mode. If not working:
1. Verify BeamCommander is listening on port 9000 (check console)
2. Test with oscsend: `oscsend localhost 9000 /laser/brightness f 0.5`
3. Check firewall/network settings

### Configuration Not Loading
Ensure you've run the application in normal mode at least once to create config files in `bin/data/`. Headless mode requires existing configuration.

## Examples

### Basic Headless Setup
```bash
# Build the application
./build.sh

# Run once with UI to configure
./start_server.sh
# Configure laser hardware, save settings, exit

# Run in headless mode
./start_headless.sh

# Access preview in browser
open http://localhost:8080
```

### Remote Control via OSC
```bash
# Terminal 1: Start headless
./start_headless.sh

# Terminal 2: Send commands
oscsend localhost 9000 /laser/shape s circle
oscsend localhost 9000 /laser/color s red
oscsend localhost 9000 /laser/brightness f 1.0
oscsend localhost 9000 /move/mode s circle
oscsend localhost 9000 /move/speed f 2.0
```

### With Open Stage Control
```bash
# Terminal 1: Start headless BeamCommander
./start_headless.sh

# Terminal 2: Start Open Stage Control
./start_open-stage-control.sh

# Access OSC UI in browser at http://localhost:8081
# Access laser preview at http://localhost:8080
```

## Future Enhancements

Possible future improvements:
- WebSocket support for lower latency updates
- Bidirectional control from web interface
- Video recording/streaming of laser output
- Multi-client preview support
- Configuration API endpoints
- Authentication/security for web access
