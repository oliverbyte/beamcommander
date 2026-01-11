---
layout: default
title: Features - BeamCommander
description: Comprehensive overview of BeamCommander features including real-time laser control, MIDI integration, OSC API, and more
permalink: /features/
---

<div class="docs-content">
  <h1>Features</h1>
  
  <p>BeamCommander provides a comprehensive laser control system with professional features for live performances and installations.</p>

  <h2>Control Methods</h2>

  <h3>MIDI Controller (Akai APC40)</h3>
  <p>Physical hardware control for tactile, real-time performance:</p>
  <ul>
    <li><strong>16 Knobs</strong> - Control brightness, position, shape scale, rotation, wave parameters, and more</li>
    <li><strong>Grid Buttons</strong> - 16 cue memory slots for instant preset recall</li>
    <li><strong>Side Buttons</strong> - Shape selection and movement pattern triggers</li>
    <li><strong>Automatic Detection</strong> - Plug and play - no configuration required</li>
    <li><strong>LED Feedback</strong> - Visual confirmation of active settings</li>
  </ul>

  <h3>Web Interface (Open Stage Control)</h3>
  <p>Control from any device on your network:</p>
  <ul>
    <li><strong>Touch-Friendly</strong> - Optimized for tablets and smartphones</li>
    <li><strong>Pre-Configured</strong> - Ready-to-use interface layouts included</li>
    <li><strong>Network Control</strong> - Access from iPad, Android, or any web browser</li>
    <li><strong>Customizable</strong> - Modify the interface to your needs</li>
  </ul>

  <h3>OSC API</h3>
  <p>Complete programmatic control for custom integrations:</p>
  <ul>
    <li><strong>UDP Port 9000</strong> - Standard OSC protocol</li>
    <li><strong>Low Latency</strong> - Real-time response for live performance</li>
    <li><strong>Extensive Commands</strong> - Control every aspect of laser output</li>
    <li><strong>Integration Ready</strong> - Works with Max/MSP, TouchDesigner, Processing, and more</li>
  </ul>

  <h2>Laser Output Features</h2>

  <h3>Shape Generation</h3>
  <ul>
    <li><strong>Basic Shapes</strong> - Line, circle, triangle, square</li>
    <li><strong>Wave Patterns</strong> - Sine waves with adjustable frequency, amplitude, and phase</li>
    <li><strong>Static & Animated</strong> - Both fixed and rotating waves</li>
    <li><strong>Scalable</strong> - Adjust size from minimal to full canvas</li>
    <li><strong>Rotatable</strong> - Continuous rotation at adjustable speeds</li>
    <li><strong>Dotted Effects</strong> - Variable dot density for segmented looks</li>
  </ul>

  <h3>Color Control</h3>
  <ul>
    <li><strong>Named Colors</strong> - Red, green, blue, and more preset colors</li>
    <li><strong>RGB Control</strong> - Full 24-bit color mixing</li>
    <li><strong>Rainbow Effects</strong> - Spatial color distribution across shapes</li>
    <li><strong>Rainbow Animation</strong> - Smooth color transitions at adjustable speeds</li>
    <li><strong>Color Blending</strong> - Adjustable gradient smoothness</li>
  </ul>

  <h3>Movement Patterns</h3>
  <ul>
    <li><strong>Pan & Tilt</strong> - Linear horizontal and vertical movement</li>
    <li><strong>Circular</strong> - Smooth circular paths</li>
    <li><strong>Figure-8</strong> - Complex infinity pattern</li>
    <li><strong>Random</strong> - Unpredictable movement for organic feels</li>
    <li><strong>Adjustable Size</strong> - Control movement amplitude</li>
    <li><strong>Variable Speed</strong> - Fast or slow, forward or reverse</li>
  </ul>

  <h3>Visual Effects</h3>
  <ul>
    <li><strong>Flash Control</strong> - Instant full brightness with fade release</li>
    <li><strong>Flicker Effect</strong> - Strobe-like visual effect at adjustable Hz</li>
    <li><strong>Brightness Control</strong> - Master dimmer from 0-100%</li>
    <li><strong>Position Control</strong> - X/Y positioning across canvas</li>
    <li><strong>Rotation</strong> - Continuous shape rotation</li>
  </ul>

  <h2>Advanced Features</h2>

  <h3>Cue System</h3>
  <ul>
    <li><strong>16 Memory Slots</strong> - Save and recall complete laser states</li>
    <li><strong>Instant Recall</strong> - One-button access to saved looks</li>
    <li><strong>Comprehensive Storage</strong> - Saves shape, color, movement, effects</li>
    <li><strong>Live Performance</strong> - Perfect for pre-programmed sequences</li>
  </ul>

  <h3>Multi-Laser Support</h3>
  <ul>
    <li><strong>Multiple DACs</strong> - Control several laser outputs simultaneously</li>
    <li><strong>Zone Mapping</strong> - Assign different zones to different lasers</li>
    <li><strong>Perspective Correction</strong> - Geometric correction for each output</li>
    <li><strong>Independent Control</strong> - Each laser can have different settings</li>
  </ul>

  <h3>Hardware Compatibility</h3>
  <p>Supports multiple DAC types through ofxLaser:</p>
  <ul>
    <li><strong>EtherDream</strong> - Industry standard Ethernet DAC ✅ Tested</li>
    <li><strong>Helios</strong> - High-performance USB DAC</li>
    <li><strong>LaserDock</strong> - USB and Ethernet projector system</li>
    <li><strong>LaserCube</strong> - Compact wireless laser projector</li>
    <li><strong>Riya</strong> - Multi-channel USB DAC</li>
    <li><strong>ILDA Support</strong> - Standard ILDA test patterns</li>
  </ul>

  <h2>Wave Pattern Features</h2>
  <ul>
    <li><strong>Wave Frequency</strong> - Control number of cycles across width (min 0.1)</li>
    <li><strong>Wave Amplitude</strong> - Adjust wave height from 0-100%</li>
    <li><strong>Wave Speed</strong> - Animate wave phase rotation</li>
    <li><strong>Static & Dynamic</strong> - Choose fixed or animated waves</li>
  </ul>

  <h2>Rainbow Effect Features</h2>
  <ul>
    <li><strong>Amount Control</strong> - Spatial color distribution from tight to full-shape</li>
    <li><strong>Speed Control</strong> - Animation speed forward/reverse</li>
    <li><strong>Blend Control</strong> - Color transition smoothness</li>
    <li><strong>Combines with Shapes</strong> - Works with all shape types</li>
  </ul>

  <h2>Technical Specifications</h2>
  <ul>
    <li><strong>Platform</strong> - macOS 15.6.1 or later</li>
    <li><strong>Framework</strong> - Built on OpenFrameworks and ofxLaser</li>
    <li><strong>Protocol</strong> - OSC (Open Sound Control) via UDP</li>
    <li><strong>Port</strong> - UDP 9000 (default)</li>
    <li><strong>Latency</strong> - Low-latency real-time processing</li>
    <li><strong>Open Source</strong> - Full source code available</li>
  </ul>

  <h2>Use Cases</h2>
  <ul>
    <li>Live VJ performances and club shows</li>
    <li>Interactive art installations</li>
    <li>Museum exhibits and galleries</li>
    <li>Concert visual effects</li>
    <li>Theater productions</li>
    <li>Corporate events and presentations</li>
    <li>Festival stages and outdoor shows</li>
    <li>Educational demonstrations</li>
  </ul>

  <div style="text-align: center; margin-top: 3rem;">
    <a href="{{ '/installation' | relative_url }}" class="btn btn-primary">Get Started</a>
    <a href="{{ '/docs' | relative_url }}" class="btn btn-secondary">Read Documentation</a>
  </div>
</div>
