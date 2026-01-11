---
layout: default
title: Documentation - BeamCommander
description: Complete documentation for BeamCommander laser control system including OSC API reference and MIDI controller mapping
permalink: /docs/
---

<div class="docs-content">
  <h1>Documentation</h1>
  
  <p>Complete guide to using and controlling BeamCommander.</p>

  <h2>Quick Start</h2>
  
  <ol>
    <li><a href="{{ '/installation' | relative_url }}">Install BeamCommander</a> for macOS</li>
    <li>Launch the application</li>
    <li>Configure your laser hardware (EtherDream, Helios, etc.)</li>
    <li>Set up output zones</li>
    <li>Start sending OSC commands or connect MIDI controller</li>
  </ol>

  <h2>OSC API Reference</h2>

  <p>BeamCommander listens for OSC commands on <strong>UDP port 9000</strong>. All commands support real-time control.</p>

  <h3>Core Laser Controls</h3>

  <h4>Shape Generation</h4>
  <pre><code>/laser/shape &lt;string&gt;</code></pre>
  <p>Set laser shape. Values: <code>line</code>, <code>circle</code>, <code>triangle</code>, <code>square</code>, <code>wave</code>, <code>staticwave</code></p>

  <h4>Color Control</h4>
  <pre><code>/laser/color &lt;string|rgb&gt;</code></pre>
  <p>Set laser color:</p>
  <ul>
    <li><strong>Named colors</strong>: <code>"blue"</code>, <code>"red"</code>, <code>"green"</code></li>
    <li><strong>RGB values</strong>: <code>r g b</code> as floats [0..1] or bytes [0..255]</li>
  </ul>

  <h4>Brightness & Visual Effects</h4>
  <pre><code>/laser/brightness &lt;float|int&gt;</code></pre>
  <p>Master brightness [0..1] or [0..255]. Alias: <code>/laser/master/brightness</code></p>

  <pre><code>/laser/dotted &lt;float|int&gt;</code></pre>
  <p>Dot pattern intensity [0..1] or [0..255]. 0 = invisible, 1 = solid line</p>

  <pre><code>/laser/flicker &lt;hz&gt;</code></pre>
  <p>Visual flicker rate. 0 = disabled, &gt;0 = flicker frequency in Hz. Alias: <code>/laser/scanrate</code></p>

  <h4>Positioning & Scaling</h4>
  <pre><code>/laser/position &lt;x&gt; &lt;y&gt;</code></pre>
  <p>Set laser position. Both values [-1..+1]. Individual: <code>/laser/position/x</code>, <code>/laser/position/y</code></p>

  <pre><code>/laser/shape/scale &lt;float&gt;</code></pre>
  <p>Shape scale factor [-1..+1]</p>

  <pre><code>/laser/rotation/speed &lt;float&gt;</code></pre>
  <p>Rotation speed in rotations/sec. Negative = reverse, 0 = static</p>

  <h3>Wave Pattern Controls</h3>
  <pre><code>/laser/wave/frequency &lt;float&gt;</code></pre>
  <p>Wave cycles across width (min 0.1)</p>

  <pre><code>/laser/wave/amplitude &lt;float&gt;</code></pre>
  <p>Wave height [0..1] as fraction of half-height</p>

  <pre><code>/laser/wave/speed &lt;float&gt;</code></pre>
  <p>Wave phase rotation speed (rotations/sec)</p>

  <h3>Rainbow Effects</h3>
  <pre><code>/laser/rainbow/amount &lt;float&gt;</code></pre>
  <p>Spatial color distribution [0..1]. 0 = many cycles, 1 = whole shape one color</p>

  <pre><code>/laser/rainbow/speed &lt;float&gt;</code></pre>
  <p>Rainbow animation speed [-1..+1]. 0 = stopped, positive = forward, negative = reverse</p>

  <pre><code>/laser/rainbow/blend &lt;float&gt;</code></pre>
  <p>Color transition smoothness [0..1]. 0 = hard steps, 1 = smooth gradient</p>

  <h3>Movement Patterns</h3>
  <pre><code>/move/mode &lt;string&gt;</code></pre>
  <p>Set movement pattern. Values: <code>none</code>, <code>off</code>, <code>circle</code>, <code>pan</code>, <code>tilt</code>, <code>eight</code>, <code>figure8</code>, <code>8</code>, <code>random</code></p>

  <pre><code>/move/size &lt;float|int&gt;</code></pre>
  <p>Movement amplitude [0..1] or [0..255]. 0 = no movement, 1 = full canvas range</p>

  <pre><code>/move/speed &lt;float&gt;</code></pre>
  <p>Movement speed in cycles/sec. Negative = reverse direction</p>

  <h3>Flash Controls</h3>
  <pre><code>/flash &lt;int&gt;</code></pre>
  <p>Flash button control. 1 = press (full brightness), 0 = release</p>

  <pre><code>/flash/release_ms &lt;int&gt;</code></pre>
  <p>Flash release fade time [0..60000] milliseconds. 0 = instant return</p>

  <h3>Cue System</h3>
  <pre><code>/cue/save</code></pre>
  <p>Arm cue saving mode (next <code>/cue/&lt;n&gt;</code> will save)</p>

  <pre><code>/cue/&lt;n&gt;</code></pre>
  <p>Save or recall cue slot (n = 1..16). If save armed: store current state. Otherwise: recall cue.</p>

  <h4>Saved Parameters</h4>
  <ul>
    <li>Shape, color (named/RGB)</li>
    <li>Movement patterns and settings</li>
    <li>Wave parameters</li>
    <li>Rainbow effects</li>
    <li>Rotation, scale, position</li>
    <li>Dotted amount, flicker rate</li>
  </ul>

  <h4>Not Saved</h4>
  <ul>
    <li>Master brightness</li>
    <li>Flash settings and button state</li>
  </ul>

  <h2 id="midi-controller">MIDI Controller Reference (Akai APC40)</h2>

  <p>Complete hardware control mapping for Akai APC40 MK2.</p>

  <h3>Setup</h3>
  <ol>
    <li>Connect Akai APC40 MK2 via USB</li>
    <li>Launch BeamCommander</li>
    <li>Controller is automatically detected</li>
    <li>LED lights confirm active connection</li>
  </ol>

  <h3>Knobs (Continuous Controllers)</h3>
  
  <h4>Top Row - Shape & Color Controls</h4>
  <ul>
    <li><strong>Knob 1</strong>: Master brightness [0..1]</li>
    <li><strong>Knob 2</strong>: Shape scale [-1..+1]</li>
    <li><strong>Knob 3</strong>: Rotation speed (rotations/sec)</li>
    <li><strong>Knob 4</strong>: Horizontal position [-1..+1]</li>
    <li><strong>Knob 5</strong>: Vertical position [-1..+1]</li>
    <li><strong>Knob 6</strong>: Dot pattern intensity [0..1]</li>
    <li><strong>Knob 7</strong>: Visual flicker rate (Hz)</li>
    <li><strong>Knob 8</strong>: RGB color mixing</li>
  </ul>

  <h4>Bottom Row - Wave & Movement Controls</h4>
  <ul>
    <li><strong>Knob 9</strong>: Wave frequency</li>
    <li><strong>Knob 10</strong>: Wave amplitude [0..1]</li>
    <li><strong>Knob 11</strong>: Wave phase rotation speed</li>
    <li><strong>Knob 12</strong>: Movement amplitude [0..1]</li>
    <li><strong>Knob 13</strong>: Movement speed (cycles/sec)</li>
    <li><strong>Knob 14</strong>: Rainbow spatial distribution</li>
  </ul>

  <h3>Grid Buttons</h3>
  <p>The 8x5 grid provides 16 cue memory slots (using bottom 4 rows, first 4 columns):</p>
  <ul>
    <li><strong>Press</strong>: Recall saved cue</li>
    <li><strong>Save Mode</strong>: Hold designated save button, then press slot to save</li>
  </ul>

  <h3>Side Buttons</h3>
  <ul>
    <li><strong>Shape Selection</strong>: Trigger different shape types</li>
    <li><strong>Movement Patterns</strong>: Activate movement modes</li>
    <li><strong>Effect Toggles</strong>: Enable/disable visual effects</li>
  </ul>

  <h2>Examples</h2>

  <h3>Example 1: Blue Circle with Medium Brightness</h3>
  <pre><code>/laser/shape circle
/laser/color blue
/laser/brightness 0.5</code></pre>

  <h3>Example 2: Moving Rainbow Wave</h3>
  <pre><code>/laser/shape wave
/laser/wave/frequency 2.0
/laser/rainbow/amount 0.8
/laser/rainbow/speed 0.5
/move/mode circle
/move/size 0.6
/move/speed 1.2</code></pre>

  <h3>Example 3: Flash Effect with Fade</h3>
  <pre><code>/flash/release_ms 2000
/flash 1
# ... wait ...
/flash 0</code></pre>

  <h3>Example 4: Save and Recall Cue</h3>
  <pre><code># Set up your desired look
/laser/shape triangle
/laser/color red
/move/mode figure8

# Save as cue 5
/cue/save
/cue/5

# Later: recall cue 5
/cue/5</code></pre>

  <h2>Troubleshooting</h2>

  <h3>No Laser Output</h3>
  <ul>
    <li>Verify DAC hardware is connected and powered</li>
    <li>Check laser safety interlock is closed</li>
    <li>Ensure laser projector is powered on</li>
    <li>Verify zone configuration in BeamCommander</li>
    <li>Check brightness is not at 0</li>
  </ul>

  <h3>MIDI Controller Not Detected</h3>
  <ul>
    <li>Reconnect USB cable</li>
    <li>Restart BeamCommander</li>
    <li>Check MIDI device appears in macOS Audio MIDI Setup</li>
    <li>Try a different USB port</li>
  </ul>

  <h3>OSC Commands Not Working</h3>
  <ul>
    <li>Verify sending to <code>localhost:9000</code></li>
    <li>Check firewall settings allow UDP port 9000</li>
    <li>Confirm OSC message format matches documentation</li>
    <li>Use OSC debugging tool to verify messages are sent</li>
  </ul>

  <h2>Performance Tips</h2>
  <ul>
    <li>Use MIDI controller for tactile, real-time control</li>
    <li>Pre-program cues for complex sequences</li>
    <li>Test your setup before live performance</li>
    <li>Keep spare cables and USB connections</li>
    <li>Monitor CPU usage for complex patterns</li>
  </ul>

  <h2>Safety</h2>
  <ul>
    <li>Never point lasers at people, aircraft, or vehicles</li>
    <li>Follow local laser safety regulations</li>
    <li>Use appropriate safety equipment</li>
    <li>Ensure audience safety at all times</li>
    <li>Have emergency stop procedures in place</li>
  </ul>

  <div style="text-align: center; margin-top: 3rem;">
    <a href="{{ '/installation' | relative_url }}" class="btn btn-primary">Installation Guide</a>
    <a href="{{ '/features' | relative_url }}" class="btn btn-secondary">See All Features</a>
  </div>
</div>
