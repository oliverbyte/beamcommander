---
layout: default
title: Installation - BeamCommander
description: Step-by-step installation guide for BeamCommander on macOS
permalink: /installation/
---

<div class="docs-content">
  <h1>Installation Guide</h1>
  
  <p>BeamCommander is available for macOS. Follow the steps below to get started.</p>

  <h2>macOS Installation</h2>
  
  <h3>Option 1: Download Release Binary (Recommended)</h3>
  <ol>
    <li>Download the latest <code>BeamCommander.app</code> from the <a href="https://github.com/{{ site.repository }}/releases" target="_blank" rel="noopener">Releases page</a></li>
    <li>Extract the downloaded archive</li>
    <li>Move <code>BeamCommander.app</code> to your Applications folder</li>
    <li>Double-click to launch the application</li>
  </ol>

  <h3>First Launch</h3>
  <p>On macOS, you may need to grant permission for the app to run:</p>
  <ol>
    <li>Right-click on <code>BeamCommander.app</code></li>
    <li>Select "Open" from the context menu</li>
    <li>Click "Open" in the security dialog</li>
  </ol>

  <h2>Building from Source</h2>
  
  <h3>Prerequisites</h3>
  <ul>
    <li>macOS 15.6.1 or later</li>
    <li>Xcode with Command Line Tools installed</li>
    <li>Git</li>
  </ul>

  <h3>Build Steps</h3>
  <pre><code>git clone https://github.com/{{ site.repository }}.git
cd BeamCommander
./build.sh</code></pre>

  <p>The build script will:</p>
  <ol>
    <li>Set up the OpenFrameworks project structure</li>
    <li>Compile the application</li>
    <li>Create the BeamCommander.app bundle</li>
  </ol>

  <h2>Initial Setup</h2>

  <h3>1. Run BeamCommander</h3>
  <p>Double-click <code>BeamCommander.app</code> or run from terminal.</p>

  <h3>2. Configure Laser Hardware</h3>
  <p>On first launch, you'll see the configuration interface:</p>
  <ol>
    <li><strong>Add Laser</strong> - Click to detect your DAC device (EtherDream, Helios, etc.)</li>
    <li><strong>Create Zones</strong> - Set up at least one output zone</li>
    <li><strong>Test Output</strong> - Verify laser output before performance</li>
    <li><strong>Save Settings</strong> - Configuration is saved automatically</li>
  </ol>

  <h2>Control Options</h2>

  <h3>Option A: MIDI Controller (Akai APC40)</h3>
  <ol>
    <li>Connect your Akai APC40 via USB</li>
    <li>Launch BeamCommander - controller is automatically detected</li>
    <li>LED lights illuminate to confirm connection</li>
    <li>Use knobs and buttons for real-time control</li>
  </ol>
  <p>See the <a href="{{ '/docs' | relative_url }}#midi-controller">MIDI Controller Reference</a> for complete button/knob mappings.</p>

  <h3>Option B: Open Stage Control Web Interface</h3>
  <ol>
    <li>Install <a href="https://openstagecontrol.ammd.net/" target="_blank" rel="noopener">Open Stage Control</a> on your device</li>
    <li>Load the provided configuration files:
      <ul>
        <li><code>open-stage-control-server.config</code> - Server settings</li>
        <li><code>open-stage-control-session.json</code> - Touch interface layout</li>
      </ul>
    </li>
    <li>Access the web interface from any device on your network</li>
    <li>Control lasers from iPad, tablet, smartphone, or desktop browser</li>
  </ul>

  <h3>Option C: Custom OSC Client</h3>
  <p>Integrate BeamCommander with your own software:</p>
  <ul>
    <li>Send OSC commands to <code>localhost:9000</code></li>
    <li>Use any OSC-compatible software (Max/MSP, TouchDesigner, Processing, etc.)</li>
    <li>See the <a href="{{ '/docs' | relative_url }}">Documentation</a> for complete OSC API reference</li>
  </ul>

  <h2>Compatible Hardware</h2>
  
  <p>BeamCommander supports a wide range of laser DAC hardware:</p>
  
  <h3>Tested Hardware</h3>
  <ul>
    <li><strong>EtherDream</strong> - Industry-standard Ethernet laser DAC ✅</li>
  </ul>

  <h3>Supported Hardware (via ofxLaser)</h3>
  <ul>
    <li><strong>Helios</strong> - High-performance USB laser DAC</li>
    <li><strong>LaserDock</strong> - USB and Ethernet laser projector system</li>
    <li><strong>LaserCube</strong> - Compact wireless laser projector</li>
    <li><strong>Riya</strong> - USB laser DAC with multiple output channels</li>
    <li><strong>ILDA</strong> - Standard ILDA test patterns and protocols</li>
  </ul>
  
  <p><strong>Note:</strong> Only EtherDream DAC has been tested. Other DACs are supported by ofxLaser but may require additional setup.</p>

  <h2>Prerequisites</h2>
  <ul>
    <li>macOS 15.6.1 or later</li>
    <li>Compatible Laser DAC hardware (EtherDream recommended)</li>
    <li>Laser projector connected to DAC</li>
    <li>Optional: Akai APC40 MIDI controller</li>
    <li>Optional: Device with web browser for Open Stage Control</li>
  </ul>

  <h2>What's Included</h2>
  <ul>
    <li><strong>BeamCommander.app</strong> - Main application</li>
    <li><strong>Configuration Files</strong> - Pre-configured for immediate use</li>
    <li><strong>Open Stage Control Templates</strong> - Ready-to-use web interface layouts</li>
    <li><strong>Documentation</strong> - Complete OSC API reference and MIDI mapping</li>
  </ul>

  <h2>Next Steps</h2>
  <ol>
    <li>Download BeamCommander from the <a href="https://github.com/{{ site.repository }}/releases">Releases page</a></li>
    <li>Follow the <a href="{{ '/installation' | relative_url }}">Installation Guide</a></li>
    <li>Configure your laser hardware</li>
    <li>Start creating laser visuals!</li>
  </ol>

  <div style="text-align: center; margin-top: 3rem;">
    <a href="https://github.com/{{ site.repository }}/releases" class="btn btn-primary">Download Now</a>
  </div>
</div>
