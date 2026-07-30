# BeamCommander - Developer Documentation

## Project Overview

BeamCommander is a comprehensive laser control system built on modified versions of OpenFrameworks and ofxLaser. This project requires bundled dependencies due to necessary modifications for joystick removal, ImGui integration improvements, and build system optimizations.

## Technical Specifications

### System Requirements
- **Operating System**: macOS 15.6.1 (Sequoia) or later
- **Architecture**: x86_64 (Intel) or ARM64 (Apple Silicon) 
- **Compiler**: Apple Clang 16.0.0 (clang-1600.0.26.6)
- **C++ Standard**: C++17
- **Build System**: GNU Make with parallel compilation (`make -j8`)
- **CMake**: 3.28.2 (for dependencies)
- **Target**: macOS 10.15+ (deployment target)

### Dependencies and Modifications

#### OpenFrameworks v0.12.0 (Master Branch - Modified)
**Base Version**: OpenFrameworks 0.12.0 (master branch as of October 2023)
**Location**: `openframeworks-src-master/`
**Version Details**: 
- Core: v0.12.0 (OF_VERSION_MAJOR=0, OF_VERSION_MINOR=12, OF_VERSION_PATCH=0)
- Branch: master with "master" pre-release tag
- Integration Date: ~October 2023
**Modifications**:
- Architecture compatibility fixes for ARM64/x86_64
- Build system optimization for parallel compilation
- Stability improvements for ImGui integration
- Custom addon integration paths

**Why Bundled**: Core framework modifications required for stable ImGui integration and build optimization cannot be achieved through standard addon approaches.

#### ofxLaser of_11.0.2 Branch (Modified)
**Base Version**: ofxLaser of_11.0.2 branch (legacy version for OpenFrameworks v0.11.x)
**Original Source**: https://github.com/sebleedelisle/ofxLaser/tree/of_11.0.2
**License**: MIT License (Copyright (c) 2012-2019 Seb Lee-Delisle)
**Location**: `openframeworks-src-master/addons/ofxLaser/`
**Version Details**:
- Branch: of_11.0.2 (legacy branch for OF 0.11.x compatibility)
- Features: Core laser control, DAC support, shape optimization, multi-laser management
- Integration Date: Based on of_11.0.2 branch, integrated with BeamCommander modifications
**Major Modifications**:
- **Joystick Support Removal**: Complete elimination of ofxJoystick dependencies
  - Removed all `HAVE_OFXJOYSTICK` conditional compilation
  - Eliminated `JoystickCueMap` and related data structures
  - Cleaned joystick configuration and mapping systems
- **ImGui Stability Improvements**: Enhanced GLFW backend safety
  - Added exception handling in ImGui shutdown procedures
  - Improved context management and resource cleanup
  - Safe exit procedures with logging
- **Build System Updates**: Parallel compilation support and dependency resolution
- **Exit Safety**: Selective JSON saving to prevent crash-inducing operations while preserving legitimate data persistence

**Why Bundled**: Extensive architectural changes including complete joystick removal and ImGui backend modifications make this incompatible with standard ofxLaser releases.

#### Additional Addons
- **ofxOsc**: OSC message handling (standard)
- **ofxMidi**: MIDI controller support (standard)  
- **ofxOpenCv**: Computer vision operations (standard)
- **ofxNetwork**: Network communication (standard)

## Project Structure

```
BeamCommander/
├── build.sh                          # Build script (compile application)
├── start_server.sh                   # Start BeamCommander laser control server
├── start_open-stage-control.sh       # Start web control interface
├── README.md                         # User documentation
├── DEVELOPER.md                      # This file
├── doc/                              # Additional documentation
│   ├── APC40 colour codes.webp
│   ├── BeamCommander APC40 Mapping.png
│   └── BeamCommander Maschine Mapping.jpg
└── openframeworks-src-master/        # Modified OpenFrameworks
    ├── addons/
    │   ├── ofxLaser/                 # Modified laser control addon
    │   └── [other standard addons]
    ├── apps/myApps/BeamCommander/    # Main application
    │   ├── src/                      # C++ source code
    │   ├── bin/                      # Compiled application
    │   ├── OSC_CONTROLS.md          # OSC command reference
    │   ├── open-stage-control-*      # UI configuration files
    │   └── Makefile                  # Build configuration
    └── libs/                         # OpenFrameworks libraries
```

## Building from Source

### Prerequisites
1. **Xcode Command Line Tools**
   ```bash
   xcode-select --install
   ```

2. **OpenFrameworks Dependencies**
   ```bash
   cd openframeworks-src-master/scripts/osx
   ./download_libs.sh
   ```

### Build Process

1. **Quick Build (Recommended)**
   ```bash
   ./build.sh              # Builds with parallel compilation from project root
   ./build.sh --clean      # Clean build cache only
   ./build.sh --clean      # Clean before building (if other args present)
   ./build.sh --help       # Show usage information
   ```

2. **Manual Build Process**
   ```bash
   cd openframeworks-src-master/apps/myApps/BeamCommander
   make clean    # Clean previous build (if needed)
   make -j8      # Compile with parallel processing
   ```

#### Build Script Options

The `build.sh` script supports the following options:
- **No arguments**: Standard build with parallel compilation  
- **`--clean`**: When used alone, cleans build cache and exits
- **`--clean` with other args**: Cleans build cache before building
- **`--help` or `-h`**: Shows usage information

**Note**: Use `--clean` to resolve build issues caused by cached dependency files with incorrect paths.

3. **Run Application**
   ```bash
   # Start both components (recommended)
   ./start_server.sh                 # Terminal 1: BeamCommander server
   ./start_open-stage-control.sh     # Terminal 2: Web interface
   
   # OR start components individually
   ./start_server.sh                 # Just BeamCommander server
   ./start_open-stage-control.sh     # Just web interface
   
   # OR run directly
   cd openframeworks-src-master/apps/myApps/BeamCommander
   ./bin/BeamCommander.app/Contents/MacOS/BeamCommander  # Direct execution
   ```

### Build Optimizations

- **Parallel Compilation**: Always use `make -j8` instead of `make` alone
- **Release Mode**: Default build target optimized for performance  
- **Architecture Detection**: Automatic x86_64/ARM64 compatibility
- **Link-Time Optimization**: Enabled for release builds

## Code Architecture

### Main Components

#### ofApp.cpp/h - Application Core
- **Setup**: Hardware initialization, addon configuration
- **Update**: Real-time processing loop, OSC message handling
- **Draw**: Laser output generation and ImGui interface
- **Exit**: Safe cleanup with selective JSON saving

#### Key Classes
- **ofxLaserManager**: Central laser control and hardware abstraction
- **OSC Message Handlers**: Command processing and routing
- **MIDI Mapper**: APC40 controller integration
- **ImGui Interface**: Real-time configuration and monitoring

### Key Modifications Made

#### Joystick Elimination (Complete)
```cpp
// REMOVED: All joystick-related code
#ifdef HAVE_OFXJOYSTICK  // These blocks eliminated
#include "ofxJoystick.h" // Dependencies removed
```

#### Safe Exit Implementation
```cpp
void ofApp::exit() {
    ofLogNotice() << "Exit called - performing safe cleanup";
    
    try {
        // Save cues to disk safely (selective saving to prevent crash-inducing operations)
        if(state) {
            saveCuesToDisk();
            ofLogNotice() << "Cues saved successfully during exit";
        }
    } catch(const std::exception& e) {
        ofLogError() << "Error saving cues during exit: " << e.what();
    } catch(...) {
        ofLogError() << "Unknown error saving cues during exit";
    }
    
    try {
        // Clean shutdown of MIDI mapper
        if(midiMapper) {
            midiMapper.reset();
            ofLogNotice() << "MIDI mapper cleaned up";
        }
    } catch(...) {
        ofLogError() << "Error cleaning up MIDI mapper";
    }
    
    try {
        // Close OSC receiver and disable laser output safely
        osc.stop();
        laser.setUseLaserFromSettings(false);
        ofLogNotice() << "OSC and laser output stopped safely";
    } catch(...) {
        ofLogError() << "Error during component cleanup";
    }
    
    ofLogNotice() << "Safe exit cleanup completed";
}
```

#### ImGui Backend Safety
```cpp
void EngineGLFW::exit() {
    ofLogNotice() << "EngineGLFW::exit() - Starting ImGui shutdown";
    try {
        // Safe ImGui cleanup with exception handling
        if (ImGui::GetCurrentContext()) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    } catch(const std::exception& e) {
        ofLogError() << "ImGui shutdown error: " << e.what();
    }
}
```

## OSC Command System

### Core Communication
- **Receive Port**: UDP 9000
- **Protocol**: OSC (Open Sound Control)
- **Integration**: Open Stage Control web interface
- **MIDI Bridge**: APC40 controller mapping

### Command Categories
- **Laser Control**: Shape, color, brightness, position
- **Movement**: Patterns, speed, size control  
- **Effects**: Rainbow, dotted patterns, rotation
- **Cues**: Preset sequences and momentary triggers
- **System**: Blackout, hold functions, axis controls

See `OSC_CONTROLS.md` for complete command reference.

## Hardware Integration

### Supported Laser DACs
- **EtherDream**: Ethernet-based, IP 10.0.1.188:45456
- **Helios**: USB-based laser controller
- **LaserDock/LaserCube**: USB-based, consumer-grade options

### MIDI Controllers  
- **Akai APC40**: Primary controller with pre-mapped functions
- **Custom Mapping**: JSON-based configuration system
- **Real-time**: Low-latency MIDI-to-OSC bridge

## Development Workflow

### Code Changes
1. Modify source files in `src/`
2. Build and test: `./build.sh` (from project root)
3. Run and test: `./start_server.sh` and `./start_open-stage-control.sh` (starts full system)
4. Check logs for errors or warnings

**Alternative development commands:**
```bash
cd openframeworks-src-master/apps/myApps/BeamCommander
make -j8                    # Build only
./bin/BeamCommander.app/Contents/MacOS/BeamCommander  # Run without web UI
```

### ImGui Interface Development
- Live interface editing through ImGui
- Configuration persistence through JSON
- Real-time parameter adjustment

### OSC Command Development
- Add handlers in `ofApp::oscReceived()`
- Test via Open Stage Control interface
- Document new commands in `OSC_CONTROLS.md`

## Troubleshooting Development Issues

### Common Build Errors
- **Missing Dependencies**: Run `download_libs.sh` script
- **Architecture Mismatch**: Clean build and recompile
- **Parallel Build Failures**: Use `make clean` then `make -j8`

### Runtime Issues
- **Segmentation Faults**: Check ImGui context management
- **Network Errors**: Verify laser DAC IP configuration
- **MIDI Problems**: Check device connections and permissions

### Memory Management
- **ImGui Contexts**: Ensure proper initialization/cleanup
- **OSC Buffers**: Monitor message queue sizes
- **Laser Buffers**: Check point generation and transmission rates

## Performance Optimization

### Real-time Considerations
- **Frame Rate**: Maintain 60fps for smooth laser output
- **OSC Latency**: <10ms typical response time
- **Memory Usage**: Monitor for leaks in long-running sessions
- **CPU Usage**: Optimize draw loops and update cycles

### Build Optimizations
- **Compiler Flags**: `-Os -mtune=native` for release
- **Link-Time Optimization**: Enabled for smaller binaries
- **Parallel Compilation**: 8-core compilation support

## Deployment

### Distribution Methods

#### 1. Homebrew (Recommended for Users)
BeamCommander is installable via Homebrew using a custom tap:

```bash
brew tap oliverbyte/beamcommander https://github.com/oliverbyte/beamcommander
brew install beamcommander
```

**For Maintainers**: See [HOMEBREW.md](HOMEBREW.md) for formula maintenance and release procedures.

#### 2. Pre-built Application Bundle
- **Bundle**: Complete `.app` bundle with all dependencies
- **Configuration**: Include default OSC/MIDI mappings
- **Documentation**: User guide and control references
- **Distribution**: Via GitHub Releases

#### 3. Source Distribution
- Users can build from source using `./build.sh`
- Requires Xcode Command Line Tools
- See [INSTALL.md](INSTALL.md) for detailed instructions

### System Requirements
- Minimum: macOS 10.15 (deployment target)
- Recommended: macOS 15.6.1+ for development
- Hardware: Any Mac with USB ports for controllers

### Release Process

When creating a new release:

1. **Update Version Numbers**: Update any version strings in the codebase
2. **Test Thoroughly**: Run all tests and manual verification
3. **Create Git Tag**: `git tag -a v1.0.0 -m "Release 1.0.0"`
4. **Update Homebrew Formula**: Update `Formula/beamcommander.rb` with new version and SHA256
5. **Create GitHub Release**: Use the release workflow or create manually
6. **Test Homebrew Installation**: Verify the formula works correctly

See [HOMEBREW.md](HOMEBREW.md) for detailed Homebrew release procedures.

## Future Development

### Planned Improvements
- Additional laser DAC support
- Web-based configuration interface
- Enhanced preset management system
- Cross-platform compatibility (Linux/Windows)

### Architecture Considerations
- Maintain modular addon structure
- Keep OSC command compatibility
- Preserve hardware abstraction layer
- Document all modifications for upstream compatibility