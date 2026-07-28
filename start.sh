#!/usr/bin/env bash
# Start BeamCommander Python server

echo "Starting BeamCommander Python server..."
echo "================================"

# Check if Python 3 is installed
if ! command -v python3 &> /dev/null; then
    echo "Error: Python 3 is not installed"
    exit 1
fi

# Check if virtual environment exists
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
fi

# Activate virtual environment
source venv/bin/activate

# Install dependencies if needed
if [ ! -f "venv/.deps_installed" ]; then
    echo "Installing dependencies..."
    pip install -r requirements.txt
    touch venv/.deps_installed
fi

# Start the server
echo ""
echo "Starting BeamCommander..."
echo "Web UI will be available at: http://localhost:8080"
echo "OSC receiver listening on port: 9000"
echo ""
echo "Press Ctrl+C to stop the server"
echo "================================"
echo ""

python3 -m beamcommander.server "$@"
