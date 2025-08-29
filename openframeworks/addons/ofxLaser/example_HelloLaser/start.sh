#!/bin/bash

# Start open-stage-control in Hintergrund und PID merken
/Applications/open-stage-control.app/Contents/MacOS/open-stage-control \
  --config-file open-stage-control-server.config \
  -n -l open-stage-control-session.json &
OPEN_STAGE_PID=$!

# make RunRelease im Vordergrund ausführen
make RunRelease

# Wenn make beendet ist, den anderen Prozess killen
kill $OPEN_STAGE_PID
