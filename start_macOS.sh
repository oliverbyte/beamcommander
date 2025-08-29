#!/bin/bash
# BeamCommander launch script (macOS)
# Builds (if needed) then launches the openFrameworks app.
# Usage: ./start_macOS.sh [--build]
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$SCRIPT_DIR"
APP_DIR="$ROOT/openframeworks/apps/myApps/BeamCommander"
APP_BIN="$APP_DIR/bin/BeamCommander.app/Contents/MacOS/BeamCommander"

DO_BUILD=false
if [[ ${1:-} == "--build" ]]; then
  DO_BUILD=true
fi

if [[ ! -x "$APP_BIN" ]] || $DO_BUILD; then
  echo "[build] Compiling BeamCommander..."
  (cd "$APP_DIR" && make -j$(sysctl -n hw.ncpu) Release) || {
    echo "Build failed" >&2; exit 1; }
fi

echo "[run] Launching BeamCommander"
"$APP_BIN" "$@"
