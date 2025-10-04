#!/bin/bash

# Start Open Stage Control web interface for BeamCommander
echo "Starting Open Stage Control web interface..."

# Navigate to BeamCommander directory for config files
cd "$(dirname "$0")/openframeworks-src-master/apps/myApps/BeamCommander"

# Check if Open Stage Control is installed
if [ ! -f "/Applications/open-stage-control.app/Contents/MacOS/open-stage-control" ]; then
    echo "Error: Open Stage Control not found in /Applications/"
    echo "Please install Open Stage Control from: https://openstagecontrol.ammd.net/"
    exit 1
fi

# Check if config files exist
if [ ! -f "open-stage-control-server.config" ]; then
    echo "Warning: open-stage-control-server.config not found"
fi

if [ ! -f "open-stage-control-session.json" ]; then
    echo "Warning: open-stage-control-session.json not found"
fi

# Start Open Stage Control
exec /Applications/open-stage-control.app/Contents/MacOS/open-stage-control \
  --config-file open-stage-control-server.config \
  -n -l open-stage-control-session.json