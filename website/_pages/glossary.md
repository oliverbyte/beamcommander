---
layout: default
title: Glossary - Laser Control & OSC Terms
description: Comprehensive glossary of laser control, OSC, MIDI, and DMX terminology for BeamCommander users. Learn the technical terms used in professional laser shows and real-time control systems.
permalink: /glossary/
---

<div class="docs-content">
  <h1>Glossary</h1>
  <p class="section-subtitle">Essential terms for laser control, OSC, MIDI, and live performance technology</p>

  <div class="glossary-grid">
    
<div class="glossary-term">
      <h3 id="osc">OSC (Open Sound Control)</h3>
      <p>A network protocol for communication between computers, synthesizers, and multimedia devices. OSC provides precise, low-latency control messages over UDP/TCP networks, making it ideal for real-time laser control and live performances.</p>
    </div>

    <div class="glossary-term">
      <h3 id="midi">MIDI (Musical Instrument Digital Interface)</h3>
      <p>Industry-standard protocol for connecting electronic musical instruments, controllers, and software. MIDI messages include note on/off, control changes (CC), and program changes, commonly used with hardware controllers like the Akai APC40 for tactile laser control.</p>
    </div>

    <div class="glossary-term">
      <h3 id="dac">DAC (Digital-to-Analog Converter)</h3>
      <p>Hardware device that converts digital laser data into analog signals for laser projectors. Popular laser DACs include EtherDream, Helios, LaserDock, and LaserCube. Each DAC has specific capabilities for points-per-second (PPS) and galvanometer control.</p>
    </div>

    <div class="glossary-term">
      <h3 id="ilda">ILDA (International Laser Display Association)</h3>
      <p>Professional organization setting standards for laser displays. The ILDA format is a file format for storing laser show data, defining points, colors, and timing information for playback on laser systems.</p>
    </div>

    <div class="glossary-term">
      <h3 id="galvanometer">Galvanometer (Galvo)</h3>
      <p>High-precision motor used in laser projectors to position mirrors that deflect the laser beam. Galvos control X and Y axis movement, enabling rapid beam positioning for creating shapes and animations. Performance measured in PPS (points per second).</p>
    </div>

    <div class="glossary-term">
      <h3 id="pps">PPS (Points Per Second)</h3>
      <p>Measurement of laser projector speed - how many individual points can be drawn per second. Higher PPS (30K+) allows for smoother animations and more complex graphics. Professional shows typically use 30,000-60,000 PPS.</p>
    </div>

    <div class="glossary-term">
      <h3 id="ilda-connector">ILDA Connector</h3>
      <p>DB-25 connector standard for analog laser control signals. Carries X/Y galvanometer position data, RGB color intensity, shutter control, and other signals between DACs and laser projectors.</p>
    </div>

    <div class="glossary-term">
      <h3 id="dmx">DMX512</h3>
      <p>Digital Multiplex protocol standard for controlling stage lighting and effects. While less common for precise laser control, DMX can control laser fixtures, RGB colors, and basic positioning on some systems.</p>
    </div>

    <div class="glossary-term">
      <h3 id="blanking">Blanking</h3>
      <p>Process of turning the laser beam off while repositioning between shapes or during beam travel. Proper blanking prevents unwanted lines and ensures safety by blocking the beam during non-display movements.</p>
    </div>

    <div class="glossary-term">
      <h3 id="interlock">Interlock</h3>
      <p>Safety mechanism that disables laser output when triggered. Required by laser safety regulations, interlocks prevent accidental exposure to laser radiation. Can be hardware (physical switch) or software-based.</p>
    </div>

    <div class="glossary-term">
      <h3 id="udp">UDP (User Datagram Protocol)</h3>
      <p>Network protocol used for fast, connectionless communication. OSC messages typically use UDP for low-latency transmission of control data. Unlike TCP, UDP doesn't guarantee delivery but offers minimal overhead for real-time applications.</p>
    </div>

    <div class="glossary-term">
      <h3 id="cc">CC (Control Change)</h3>
      <p>MIDI message type for continuous controller data like knobs, sliders, and foot pedals. CC messages range from 0-127 and control parameters like volume, pan, modulation, and custom mappings for laser effects.</p>
    </div>

    <div class="glossary-term">
      <h3 id="latency">Latency</h3>
      <p>Time delay between sending a control command and the laser system responding. Low latency (< 10ms) is critical for live performance where instant response to MIDI controllers or OSC commands is required.</p>
    </div>

    <div class="glossary-term">
      <h3 id="frame-rate">Frame Rate</h3>
      <p>Number of complete laser images displayed per second, measured in Hz or FPS. Typical laser shows run at 30-60 FPS. Higher frame rates provide smoother animations but require faster galvanometers and higher PPS.</p>
    </div>

    <div class="glossary-term">
      <h3 id="vector">Vector Graphics</h3>
      <p>Graphics defined by mathematical paths rather than pixels. Lasers naturally draw vector graphics by moving the beam along lines and curves. SVG files can be converted to laser paths for projection.</p>
    </div>

    <div class="glossary-term">
      <h3 id="rgb">RGB (Red, Green, Blue)</h3>
      <p>Color mixing model using red, green, and blue laser sources. Professional RGB laser projectors combine these three wavelengths to create full-color displays. Each color has intensity control (0-255 or 0-100%).</p>
    </div>

    <div class="glossary-term">
      <h3 id="beam-show">Beam Show</h3>
      <p>Laser display emphasizing visible laser beams in atmospheric conditions (fog, haze). Beam shows create volumetric effects and aerial displays, contrasting with graphic shows that project shapes on surfaces.</p>
    </div>

    <div class="glossary-term">
      <h3 id="safety-zone">Safety Zone</h3>
      <p>Designated area where laser beams are restricted or prohibited to prevent audience exposure. Safety zones are defined in laser show programming to ensure compliance with laser safety regulations and protect viewers.</p>
    </div>

    <div class="glossary-term">
      <h3 id="cue">Cue</h3>
      <p>Preset configuration or scene in a laser show. Cues store complete states including shapes, colors, positions, and effects. Operators trigger cues via MIDI controllers or OSC commands for instant recall during performances.</p>
    </div>

    <div class="glossary-term">
      <h3 id="vj">VJ (Video Jockey / Visual Jockey)</h3>
      <p>Live visual performer who creates and manipulates visuals in real-time, often synchronized with music. VJs use laser systems, video projections, and LED displays to enhance live events and concerts.</p>
    </div>

    <div class="glossary-term">
      <h3 id="timecode">Timecode</h3>
      <p>Synchronization protocol that provides time reference for aligning multiple systems. SMPTE, MIDI Clock, and Ableton Link are common timecode formats used to sync laser shows with music, lighting, and video.</p>
    </div>

    <div class="glossary-term">
      <h3 id="ofx">ofxLaser</h3>
      <p>OpenFrameworks addon for laser graphics and DAC communication. BeamCommander is built on ofxLaser, providing vector graphics rendering, multi-DAC support, and real-time effects for creative laser control.</p>
    </div>

    <div class="glossary-term">
      <h3 id="openframeworks">OpenFrameworks</h3>
      <p>Open-source C++ toolkit for creative coding. Provides tools for graphics, audio, video, and hardware interfacing. BeamCommander uses OpenFrameworks for cross-platform development and real-time performance.</p>
    </div>

    <div class="glossary-term">
      <h3 id="apc40">Akai APC40</h3>
      <p>Professional MIDI controller designed for Ableton Live, featuring knobs, faders, buttons, and RGB pads. Widely used for laser control due to its tactile interface and extensive mapping capabilities.</p>
    </div>

    <div class="glossary-term">
      <h3 id="etherdream">EtherDream</h3>
      <p>Popular open-source laser DAC created by Jacob Potter. Connects via Ethernet, supports 30K PPS, and is widely supported across laser software platforms. Known for reliability and affordability.</p>
    </div>

    <div class="glossary-term">
      <h3 id="helios">Helios DAC</h3>
      <p>USB laser DAC supporting up to 48K PPS with low latency. Compact design and plug-and-play operation make it popular for portable laser setups and small to medium-scale installations.</p>
    </div>

    <div class="glossary-term">
      <h3 id="laserdock">LaserDock</h3>
      <p>Consumer-grade USB laser DAC integrated with compact RGB laser projector. Affordable entry point for laser enthusiasts, supporting creative software development through open-source SDKs.</p>
    </div>

    <div class="glossary-term">
      <h3 id="scan-rate">Scan Rate</h3>
      <p>Speed at which galvanometers can move the laser beam, related to PPS. Higher scan rates enable faster animations and more complex graphics without flicker. Measured in degrees per second or maximum deflection speed.</p>
    </div>

    <div class="glossary-term">
      <h3 id="modulation">Modulation</h3>
      <p>Controlling laser intensity or color over time. Modulation creates effects like pulsing, fading, and color transitions. Can be controlled via analog signals or digital commands depending on DAC capabilities.</p>
    </div>

    <div class="glossary-term">
      <h3 id="show-file">Show File</h3>
      <p>File containing laser show data including shapes, colors, timing, and sequences. Common formats include ILDA (.ild), proprietary show formats, and BeamCommander project files with OSC command mappings.</p>
    </div>

    <div class="glossary-term">
      <h3 id="live-input">Live Input</h3>
      <p>Real-time control of laser parameters during performance using controllers, OSC, or audio reactivity. Live input enables improvisation and responsive visuals that adapt to music and audience energy.</p>
    </div>

  </div>

  <div class="glossary-footer">
    <h2>Need More Information?</h2>
    <p>Check our <a href="/docs">Documentation</a> for detailed API references and control examples, or visit the <a href="/faq">FAQ</a> for common questions.</p>
    <div class="cta-buttons">
      <a href="/installation" class="btn btn-primary">Get Started</a>
      <a href="https://github.com/oliverbyte/BeamCommander" class="btn btn-secondary">View on GitHub</a>
    </div>
  </div>

</div>

<style>
.glossary-grid {
  display: grid;
  gap: 2rem;
  margin-top: 3rem;
}

.glossary-term {
  background: var(--bg-light);
  padding: 2rem;
  border-radius: 12px;
  border: 1px solid var(--border-color);
  transition: var(--transition);
}

.glossary-term:hover {
  border-color: var(--neon-green);
  box-shadow: 0 4px 12px rgba(0, 255, 65, 0.2);
}

.glossary-term h3 {
  color: var(--neon-green);
  margin-bottom: 0.75rem;
  font-size: 1.5rem;
}

.glossary-term p {
  color: var(--text-light);
  line-height: 1.8;
}

.glossary-footer {
  margin-top: 4rem;
  text-align: center;
  padding: 3rem;
  background: var(--bg-light);
  border-radius: 12px;
  border: 2px solid var(--neon-green);
}

.glossary-footer h2 {
  color: var(--neon-green);
  margin-bottom: 1rem;
}

.glossary-footer p {
  color: var(--text-light);
  margin-bottom: 2rem;
}

.glossary-footer a:not(.btn) {
  color: var(--neon-green);
  text-decoration: underline;
}
</style>
