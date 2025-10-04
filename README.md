# BeamCommander - Laser Control System

BeamCommander is a comprehensive laser control system that bridges OSC (Open Sound Control) commands with laser hardware, providing real-time visual effects for performances and installations.

**Live Performance Ready**: Control your lasers in real-time using an Akai APC40 MIDI controller and/or intuitive web interface. Designed specifically for live performances, VJ sets, and externally controlled laser shows via OSC commands. Perfect for artists, performers, and installation designers who need responsive, tactile control over complex laser visuals.

## Demo

![BeamCommander Demo](doc/BeamCommander_Demo.gif)

*Real-time laser control demonstration showing Open Stage Control interface integration with BeamCommander*

![BeamCommander Live Demo](doc/BeamCommander_Live_Demo.gif)

*Live performance demonstration with Akai APC40 MIDI controller and iPad (Open Stage Control browser UI) controlling laser effects in real-time*

## Quick Start (Users)

### Prerequisites
- macOS 15.6.1 or later
- [Open Stage Control](https://openstagecontrol.ammd.net/) application installed in `/Applications/`
- Akai APC40 MIDI controller (optional but recommended)
- Compatible laser DAC hardware (EtherDream, Helios, LaserDock/LaserCube)

### Running the System

1. **First Time Setup**
   ```bash
   ./build.sh    # Build the application (only needed once or after code changes)
   ```

2. **Start the System Components**
   
   **Option A: Start Both Components (Recommended)**
   ```bash
   # Terminal 1: Start BeamCommander server
   ./start_server.sh
   
   # Terminal 2: Start web interface (in separate terminal)
   ./start_open-stage-control.sh
   ```
   
   **Option B: Start Only BeamCommander**
   ```bash
   ./start_server.sh    # Start just the laser control server
   ```
   
   **Option C: Start Only Web Interface**
   ```bash
   ./start_open-stage-control.sh    # Start just the web control interface
   ```

2. **Connect Your Controllers**
   - **Web Interface**: Open your browser to the displayed URL (typically `http://localhost:8080`)
   - **MIDI Controller**: Connect your Akai APC40 via USB
   - **Laser Hardware**: Connect your laser DAC via USB or Ethernet

3. **Initial Setup**
   - Launch the desktop app first to configure your laser zones and output mapping
   - Use the ImGui interface to set up laser outputs and zone transformations
   - Save your configuration - it will persist between sessions

### Control Methods

#### Web Browser (Open Stage Control)
- Access the touch-friendly web interface from any device on your network
- Control laser shapes, colors, movement patterns, and effects
- Perfect for performance control and remote operation

#### MIDI Controller (Akai APC40)
- Physical knobs and buttons for tactile control
- Pre-mapped controls for laser brightness, position, colors, and effects
- Momentary buttons for instant cue triggering
- See `OSC_CONTROLS.md` for complete mapping details

#### Desktop Application
- Direct laser output configuration
- Zone setup and perspective correction
- Advanced mask management
- Preset system for different venues/setups

### Features

- **Multi-Laser Support**: Control multiple laser outputs simultaneously
- **Real-time OSC Control**: Low-latency command processing
- **Shape Generation**: Lines, circles, triangles, squares, wave patterns
- **Color Systems**: Static colors, RGB control, rainbow effects
- **Movement Patterns**: Pan, tilt, circular, figure-8, random movement
- **Visual Effects**: Dotted patterns, brightness control, rotation
- **Cue System**: Pre-programmed sequences and momentary triggers
- **Zone Mapping**: Perspective correction and output transformation

### Troubleshooting

- **No MIDI ports available**: Check APC40 USB connection and drivers
- **Network errors**: Verify laser DAC network settings (typically 10.0.1.188)
- **App crashes on exit**: Improved exit handling reduces crashes, but occasional exit crashes may still occur due to OpenFrameworks/ImGui cleanup
- **Open Stage Control won't start**: Verify installation path in start_open-stage-control.sh

## System Requirements

- **Operating System**: macOS 15.6.1 (Sequoia)
- **Architecture**: x86_64 (Intel) or ARM64 (Apple Silicon)
- **Compiler**: Apple Clang 16.0.0
- **Build System**: Make with parallel compilation support
- **Network**: Ethernet connection for laser DACs
- **USB**: For MIDI controllers and USB laser hardware

## Quick Reference

- `./build.sh` - Build the application (first time or after code changes)
- `./start_server.sh` - Start BeamCommander laser control server
- `./start_open-stage-control.sh` - Start web control interface
- `DEVELOPER.md` - Technical documentation for developers
- `LICENSE.md` - Complete licensing information and third-party attributions
- `openframeworks-src-master/apps/myApps/BeamCommander/OSC_CONTROLS.md` - Complete OSC command reference

## Framework Versions

BeamCommander is built on modified versions of open-source frameworks:
- **OpenFrameworks**: v0.12.0 (master branch, October 2023) - Modified for enhanced stability and build optimization
- **ofxLaser**: of_11.0.2 branch (legacy for OF 0.11.x) - Modified with joystick removal and ImGui safety improvements

For detailed modification information, see `DEVELOPER.md`.

## License

BeamCommander incorporates multiple open-source components:
- **BeamCommander application code**: MIT License
- **OpenFrameworks v0.12.0**: MIT License  
- **ofxLaser of_11.0.2 branch**: MIT License

See `LICENSE.md` for complete licensing information and third-party attributions.

## Support

For technical issues, hardware compatibility, or performance setup assistance, refer to the developer documentation in `DEVELOPER.md`.