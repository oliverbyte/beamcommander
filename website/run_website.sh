#!/bin/bash

# Run Jekyll Website locally with Docker

echo "=========================================="
echo "BeamCommander Website - Local Preview"
echo "=========================================="
echo ""

cd "$(dirname "$0")"

# Clean up any previous containers
docker ps -a | grep jekyll | awk '{print $1}' | xargs docker rm -f 2>/dev/null

echo "Starting Jekyll server with Docker..."
echo ""
echo "Website will be available at: http://localhost:4000/"
echo "Note: Local server uses baseurl='' (GitHub Pages uses '/BeamCommander')"
echo "Press Ctrl+C to stop the server"
echo ""

docker run --rm \
  --volume="$PWD:/srv/jekyll" \
  --publish 4000:4000 \
  jekyll/jekyll:latest \
  jekyll serve --watch --host 0.0.0.0 --baseurl "" --skip-initial-build
