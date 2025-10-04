#!/bin/bash

# Parse command line arguments
START_WEB_UI=true
APP_ONLY=false

for arg in "$@"; do
    case $arg in
        --app-only)
            START_WEB_UI=false
            APP_ONLY=true
            ;;
        --no-web)
            START_WEB_UI=false
            ;;
        -h|--help)
            echo "Usage: $0 [--app-only|--no-web] [--help]"
            echo ""
            echo "Start BeamCommander application with optional web interface"
            echo ""
            echo "Options:"
            echo "  (no args)     Start both BeamCommander and Open Stage Control web interface (default)"
            echo "  --app-only    Start only BeamCommander application"
            echo "  --no-web      Same as --app-only"
            echo "  --help        Show this help message"
            echo ""
            echo "Separate scripts:"
            echo "  ./start_open-stage-control.sh  Start only the web interface"
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

# Check if application is built
if [ ! -f "bin/BeamCommander.app/Contents/MacOS/BeamCommander" ]; then
    echo "BeamCommander not found! Run ./build.sh first to build the application."
    exit 1
fi

# Start Open Stage Control in background if requested
OPEN_STAGE_PID=""
if [ "$START_WEB_UI" = true ]; then
    echo "Starting Open Stage Control web interface..."
    
    # Use the separate script in background
    "$(dirname "$0")/start_open-stage-control.sh" &
    OPEN_STAGE_PID=$!
    
    # Wait a moment for Open Stage Control to start
    sleep 2
    echo "Web interface should be available at: http://localhost:8080"
fi

# Run BeamCommander in foreground
echo "Starting BeamCommander application..."
./bin/BeamCommander.app/Contents/MacOS/BeamCommander

# When BeamCommander exits, kill the open-stage-control process if it was started
if [ -n "$OPEN_STAGE_PID" ]; then
    echo "Shutting down Open Stage Control..."
    kill $OPEN_STAGE_PID 2>/dev/null
fi
