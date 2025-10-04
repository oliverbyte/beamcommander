#!/bin/bash

# Start BeamCommander application only
echo "Starting BeamCommander server..."

# Parse command line arguments
for arg in "$@"; do
    case $arg in
        -h|--help)
            echo "Usage: $0 [--help]"
            echo ""
            echo "Start BeamCommander laser control application"
            echo ""
            echo "Options:"
            echo "  --help        Show this help message"
            echo ""
            echo "Other scripts:"
            echo "  ./start_open-stage-control.sh  Start web interface separately"
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

# Run BeamCommander
echo "Starting BeamCommander application..."
echo "Note: Start web interface separately with: ./start_open-stage-control.sh"
./bin/BeamCommander.app/Contents/MacOS/BeamCommander