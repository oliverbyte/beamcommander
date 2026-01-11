---
layout: default
title: FAQ - BeamCommander
description: Frequently asked questions about BeamCommander laser control system
permalink: /faq/
---

<div class="docs-content">
  <h1>Frequently Asked Questions</h1>
  
  <p>Find answers to common questions about BeamCommander installation, configuration, and usage.</p>

  <h2>General Questions</h2>

  <h3>What is BeamCommander?</h3>
  <p>BeamCommander is a professional laser control system that bridges OSC (Open Sound Control) commands with laser hardware. It provides real-time visual effects for live performances and installations, with support for MIDI controllers and web-based interfaces.</p>

  <h3>What platforms does BeamCommander support?</h3>
  <p>Currently, BeamCommander is available for macOS 15.6.1 or later. The application is built on OpenFrameworks and can potentially be compiled for other platforms with some modifications.</p>

  <h3>Is BeamCommander free?</h3>
  <p>Yes, BeamCommander is an open-source project. The source code is freely available on GitHub for anyone to use, modify, and contribute to.</p>

  <h3>What laser hardware is compatible?</h3>
  <p>BeamCommander supports multiple DAC types through the ofxLaser framework, including EtherDream (tested), Helios, LaserDock, LaserCube, and Riya. EtherDream is the recommended and tested option.</p>

  <h2>Installation & Setup</h2>

  <h3>How do I install BeamCommander?</h3>
  <p>Download the latest release from the <a href="https://github.com/{{ site.repository }}/releases" target="_blank" rel="noopener">Releases page</a>, extract the archive, and move BeamCommander.app to your Applications folder. See the <a href="{{ '/installation' | relative_url }}">Installation Guide</a> for detailed instructions.</p>

  <h3>Do I need special hardware to use BeamCommander?</h3>
  <p>Yes, you need:</p>
  <ul>
    <li>A compatible laser DAC (EtherDream recommended)</li>
    <li>A laser projector connected to the DAC</li>
    <li>Optional: Akai APC40 MIDI controller for tactile control</li>
    <li>Optional: Device with web browser for Open Stage Control</li>
  </ul>

  <h3>How do I configure my laser hardware?</h3>
  <p>On first launch, BeamCommander opens a configuration interface where you can add laser hardware, create output zones, and test your setup. Settings are saved automatically for future sessions.</p>

  <h3>Can I use multiple lasers simultaneously?</h3>
  <p>Yes! BeamCommander supports multiple laser outputs with independent zone mapping and perspective correction for each output.</p>

  <h2>Control Methods</h2>

  <h3>What control options are available?</h3>
  <p>BeamCommander offers three control methods:</p>
  <ul>
    <li><strong>MIDI Controller</strong>: Physical control with Akai APC40</li>
    <li><strong>Web Interface</strong>: Touch control via Open Stage Control</li>
    <li><strong>OSC API</strong>: Custom integration with any OSC-compatible software</li>
  </ul>

  <h3>How do I connect my Akai APC40?</h3>
  <p>Simply plug the APC40 into your Mac via USB before launching BeamCommander. The controller is automatically detected, and LED lights will confirm the connection.</p>

  <h3>What is Open Stage Control?</h3>
  <p>Open Stage Control is a free, cross-platform OSC controller software with a web-based interface. BeamCommander includes pre-configured layouts for immediate use. Download it from <a href="https://openstagecontrol.ammd.net/" target="_blank" rel="noopener">openstagecontrol.ammd.net</a>.</p>

  <h3>Can I control BeamCommander from my iPad?</h3>
  <p>Yes! Using Open Stage Control, you can access the touch-friendly web interface from any device on your network, including iPads, tablets, and smartphones.</p>

  <h3>What OSC software is compatible?</h3>
  <p>Any software that can send OSC messages over UDP is compatible, including Max/MSP, TouchDesigner, Processing, Pure Data, and many others.</p>

  <h2>Features & Functionality</h2>

  <h3>What shapes can BeamCommander generate?</h3>
  <p>BeamCommander supports lines, circles, triangles, squares, and wave patterns (both static and animated). All shapes can be scaled, rotated, and positioned in real-time.</p>

  <h3>How do I create rainbow effects?</h3>
  <p>Use the rainbow OSC commands to control spatial color distribution (<code>/laser/rainbow/amount</code>), animation speed (<code>/laser/rainbow/speed</code>), and color blending (<code>/laser/rainbow/blend</code>).</p>

  <h3>What are cues and how do I use them?</h3>
  <p>Cues are memory slots (16 total) that store complete laser states including shape, color, movement, and effects. To save: send <code>/cue/save</code> then <code>/cue/5</code> (for slot 5). To recall: send <code>/cue/5</code>.</p>

  <h3>Can I save my settings?</h3>
  <p>Yes, hardware configuration and zone settings are automatically saved. Use the cue system to save and recall specific visual looks during performances.</p>

  <h3>What movement patterns are available?</h3>
  <p>BeamCommander supports pan, tilt, circular, figure-8, and random movement patterns, all with adjustable speed and amplitude.</p>

  <h2>Performance & Troubleshooting</h2>

  <h3>Why is there no laser output?</h3>
  <p>Check the following:</p>
  <ul>
    <li>DAC hardware is connected and powered</li>
    <li>Laser safety interlock is closed</li>
    <li>Laser projector is powered on</li>
    <li>Zone configuration is correct</li>
    <li>Brightness is not at 0</li>
  </ul>

  <h3>My MIDI controller isn't detected</h3>
  <p>Try these steps:</p>
  <ul>
    <li>Reconnect the USB cable</li>
    <li>Restart BeamCommander</li>
    <li>Check the device appears in macOS Audio MIDI Setup</li>
    <li>Try a different USB port</li>
  </ul>

  <h3>OSC commands aren't working</h3>
  <p>Verify:</p>
  <ul>
    <li>Sending to <code>localhost:9000</code> (or correct IP address)</li>
    <li>Firewall allows UDP port 9000</li>
    <li>OSC message format matches documentation</li>
    <li>Messages are being sent (use debugging tools)</li>
  </ul>

  <h3>How can I optimize performance?</h3>
  <ul>
    <li>Close unnecessary applications</li>
    <li>Use wired Ethernet for DAC connections when possible</li>
    <li>Reduce complexity of wave patterns if needed</li>
    <li>Pre-program cues for complex sequences</li>
  </ul>

  <h2>Development & Contribution</h2>

  <h3>Is the source code available?</h3>
  <p>Yes! BeamCommander is open-source. The complete source code is available on <a href="https://github.com/{{ site.repository }}" target="_blank" rel="noopener">GitHub</a>.</p>

  <h3>Can I modify BeamCommander?</h3>
  <p>Absolutely! As an open-source project, you're free to modify, extend, and customize BeamCommander to fit your needs. Contributions are welcome via pull requests.</p>

  <h3>What is ofxLaser?</h3>
  <p>ofxLaser is an OpenFrameworks addon that provides the laser DAC communication layer. It's developed by Seb Lee-Delisle and supports multiple DAC types. BeamCommander is built on top of ofxLaser.</p>

  <h3>How can I contribute to the project?</h3>
  <p>Contributions are welcome! You can:</p>
  <ul>
    <li>Report bugs and issues on GitHub</li>
    <li>Submit feature requests</li>
    <li>Contribute code via pull requests</li>
    <li>Improve documentation</li>
    <li>Share your laser show experiences</li>
  </ul>

  <h2>Safety & Legal</h2>

  <h3>Is it safe to use lasers?</h3>
  <p>Lasers can be dangerous if not used properly. Always follow laser safety regulations, never point lasers at people or aircraft, and use appropriate safety equipment. Ensure audience safety at all times.</p>

  <h3>Do I need a license to use lasers?</h3>
  <p>Laser regulations vary by country and jurisdiction. Check your local laws regarding laser use, especially for public performances. Some locations require laser safety certification.</p>

  <h3>What are the system requirements?</h3>
  <p>macOS 15.6.1 or later is required. A compatible laser DAC and projector are necessary for laser output. MIDI controller and Open Stage Control are optional.</p>

  <h2>Still Have Questions?</h2>
  
  <p>If you didn't find your answer here, please:</p>
  <ul>
    <li>Check the <a href="{{ '/docs' | relative_url }}">complete documentation</a></li>
    <li>Search <a href="https://github.com/{{ site.repository }}/issues" target="_blank" rel="noopener">GitHub Issues</a></li>
    <li>Ask in <a href="https://github.com/{{ site.repository }}/discussions" target="_blank" rel="noopener">GitHub Discussions</a></li>
    <li>Open a new issue with details about your problem</li>
  </ul>
</div>
