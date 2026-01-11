#!/usr/bin/env python3
"""
Simple preview server for Jekyll website.
This serves the static files without Jekyll processing.
For full Jekyll preview, use Docker or install Ruby/Jekyll locally.
"""

import http.server
import socketserver
import os
import sys

PORT = 4000
WEBSITE_DIR = os.path.dirname(os.path.abspath(__file__))

class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEBSITE_DIR, **kwargs)
    
    def end_headers(self):
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate')
        super().end_headers()

if __name__ == "__main__":
    os.chdir(WEBSITE_DIR)
    
    with socketserver.TCPServer(("", PORT), CustomHTTPRequestHandler) as httpd:
        print("=" * 60)
        print("BeamCommander Website Preview Server")
        print("=" * 60)
        print(f"\nServing at: http://localhost:{PORT}/")
        print(f"Base URL: http://localhost:{PORT}/BeamCommander/")
        print(f"Directory: {WEBSITE_DIR}")
        print("\nNote: This is a simple file server.")
        print("For full Jekyll processing (Liquid templates, etc.),")
        print("use Docker or install Ruby/Jekyll locally.")
        print("\nPress Ctrl+C to stop the server")
        print("=" * 60)
        print()
        
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n\nServer stopped.")
            sys.exit(0)
