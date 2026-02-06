#!/bin/bash

# Start BeamCommander in headless mode
echo "Starting BeamCommander in headless mode..."
echo ""
echo "This will run the laser control system without a UI window."
echo "All functionality is controlled via:"
echo "  - OSC commands on port 9000"
echo "  - Web preview at http://localhost:8080"
echo ""

# Parse command line arguments
for arg in "$@"; do
    case $arg in
        -h|--help)
            echo "Usage: $0 [--help]"
            echo ""
            echo "Start BeamCommander in headless mode (no window/UI)"
            echo ""
            echo "Features:"
            echo "  - Runs without graphical window"
            echo "  - Uses existing config files from bin/data/"
            echo "  - Listens for OSC commands on port 9000"
            echo "  - Serves 3D preview via HTTP on port 8080"
            echo ""
            echo "Control Methods:"
            echo "  - OSC: Send commands to localhost:9000"
            echo "  - Web Preview: Open http://localhost:8080 in browser"
            echo "  - MIDI: Connect MIDI controller (auto-detected)"
            echo ""
            echo "Other scripts:"
            echo "  ./start_server.sh                Start with UI (normal mode)"
            echo "  ./start_open-stage-control.sh    Start web control interface"
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Navigate to BeamCommander directory
cd "$(dirname "$0")/openframeworks-src-master/apps/myApps/BeamCommander"

# Check if application is built (macOS bundle structure)
# Note: This script is designed for macOS. Linux/Windows users should
# adjust the path to point to the appropriate executable location.
if [ ! -f "bin/BeamCommander.app/Contents/MacOS/BeamCommander" ]; then
    echo "BeamCommander not found! Run ./build.sh first to build the application."
    echo "Note: This script expects macOS .app bundle structure."
    exit 1
fi

# Run BeamCommander in headless mode
echo "Starting BeamCommander in headless mode..."
echo "Press Ctrl+C to stop"
echo ""
./bin/BeamCommander.app/Contents/MacOS/BeamCommander --headless
