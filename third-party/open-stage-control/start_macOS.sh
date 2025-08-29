#!/bin/bash
# open-stage-control launcher (external system install)
# Loads local server config + session layout if present beside this script.
# Usage: ./third-party/open-stage-control/start_macOS.sh [extra open-stage-control args]
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_PATH="/Applications/open-stage-control.app/Contents/MacOS/open-stage-control"
CONFIG_FILE="${SCRIPT_DIR}/open-stage-control-server.config"
SESSION_FILE="${SCRIPT_DIR}/open-stage-control-session.json"

if [[ ! -x "$APP_PATH" ]]; then
  echo "ERROR: open-stage-control not found at $APP_PATH" >&2
  echo "Install from https://github.com/jean-emmanuel/open-stage-control/releases" >&2
  exit 1
fi

CMD=("$APP_PATH" -n)
[[ -f "$CONFIG_FILE" ]] && CMD+=(--config-file "$CONFIG_FILE")
[[ -f "$SESSION_FILE" ]] && CMD+=(-l "$SESSION_FILE")

# Pass through any additional user args
if [[ $# -gt 0 ]]; then
  CMD+=("$@")
fi

echo "Launching open-stage-control: ${CMD[*]}"
exec "${CMD[@]}"
