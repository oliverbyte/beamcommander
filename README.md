# BeamCommander

An experimental laser performance and visualization environment built on openFrameworks and **ofxLaser**. BeamCommander provides:

- Realtime 2D/3D laser shape generation & animated effects
- Multi‑DAC (laser controller) output management (via ofxLaser)
- On‑screen 3D / 2D preview & calibration UI (Dear ImGui based)
- OSC server for remote control
- Optional web touchscreen / desktop control surface via bundled **open-stage-control**
- MIDI + joystick + (optionally) Native Instruments Maschine Micro + Akai APC40 mk2 cue triggering / parameter control

> WARNING: LASERS ARE DANGEROUS. Always follow proper laser safety procedures, use a physical emergency stop, respect local regulations, and never expose eyes to direct or reflected beams.

---
## Status
Prototype / in active development. Interfaces, OSC addresses and file formats may change without notice.

---
## License
BeamCommander is released under **AGPL-3.0-or-later**. See `LICENSE`. Third‑party components retain their own licenses – see `NOTICE` and `THIRD_PARTY_LICENSES/`.

Using / modifying / deploying BeamCommander over a network (including offering it as a hosted service) requires providing complete corresponding source code (AGPL clause).

---
## Supported Laser DACs (via ofxLaser)
Current transport & controller support derives from the embedded ofxLaser version. As of this snapshot the following are supported:

USB:
- Helios DAC
- LaserDock / LaserCube (USB)

Network:
- Ether Dream
- (Experimental / alpha) IDN (ILDA Digital Network) discovery (integration work in progress)

Planned / Roadmap (not yet guaranteed in this build):
- LaserCube network protocol (Wi‑Fi)
- Full IDN DacAssigner integration (Hello protocol)

All multi‑laser features (zones, masking, per-laser calibration & render profiles) are available once controllers are detected.

---
## Platforms
Currently only macOS launch scripts are provided. (Project should be portable to Linux / Windows with standard openFrameworks workflows, but no packaged scripts yet.)

Start Scripts:
- `start_macOS.sh` (root) – builds (optional `--build`) and launches the BeamCommander openFrameworks app.
- `third-party/open-stage-control/start_macOS.sh` – launches a system installed open-stage-control from `/Applications/open-stage-control.app` and auto-loads local server & session config files if present.

Notes:
- open-stage-control is NOT bundled anymore; user must install it separately.
- Pass additional flags to either script after the first argument (e.g. `./start_macOS.sh --build`).

---
## Launching
### 1. Build (first time or after code changes)
Use the standard openFrameworks build system (Xcode project or make). After building, ensure the app bundle exists at:
```
openframeworks/apps/myApps/BeamCommander/bin/BeamCommander.app
```

### 2. Start BeamCommander (native oF UI + OSC server)
From repository root:
```
./start_macOS.sh
```
This launches the BeamCommander window which contains:
- 3D / 2D laser preview & editing UI (ImGui driven)
- Laser configuration (zones, masks, calibration – provided by ofxLaser UI layer)
- Runtime parameter controls (shapes, colors, movement, effects)
- OSC server (default UDP listen port: 9000) for remote / web UI / MIDI mapping messages

### 3. Start the Web OSC Control Surface (open-stage-control)
Install open-stage-control (macOS .app) separately, then:
```
cd third-party/open-stage-control
./start_macOS.sh
```
Examples:
```
./start_macOS.sh --port 8080 --theme dark
```
The script auto-loads (if present in the same folder):
- `open-stage-control-server.config` (server settings)
- `open-stage-control-session.json` (UI layout / widgets)

Point a browser (local or remote) to the reported host/port. BeamCommander (default listen 9000) should match the destination configured in open-stage-control.

Running order is arbitrary; ensure OSC port consistency.

---
## OSC
Default incoming BeamCommander OSC port: **9000** (see `ofApp::oscPort`).

### Address Reference (current build)
All values are floats (0..1) unless noted. Integers are 32‑bit. Booleans accept 0/1 or false/true.
Prefix shorthand: `/bc` shown below; legacy aliases `/beam` may be accepted in transitional builds.

Core Shape & Color
- `/bc/shape` (int) : 0=Circle 1=Line 2=Triangle 3=Square 4=StaticWave
- `/bc/color` (int) : 0=Blue 1=Red 2=Green
- `/bc/use_custom` (bool)
- `/bc/custom_rgb` (r,g,b floats 0..1) set three channels at once

Geometry & Motion
- `/bc/pos` (x,y floats -1..1 normalized center=0)
- `/bc/pos/x` (float -1..1)
- `/bc/pos/y` (float -1..1)
- `/bc/scale` (float -1..1 mapped internally to positive scale)
- `/bc/rotation/speed` (float cycles/sec; signed)
- `/bc/move/mode` (int) : 0=None 1=Circle 2=Pan 3=Tilt 4=Eight 5=Random
- `/bc/move/speed` (float cycles/sec; signed)
- `/bc/move/size` (float 0..1)
- `/bc/motion/hold` (bool) freeze current dynamic phases

Wave (StaticWave / modulation)
- `/bc/wave/frequency` (float Hz-ish)
- `/bc/wave/amplitude` (float 0..1)
- `/bc/wave/speed` (float cycles/sec; signed)

Rainbow / Color FX
- `/bc/rainbow/speed` (float cycles/sec)
- `/bc/rainbow/amount` (float blend 0=base color 1=full rainbow)
- `/bc/rainbow/blend` (float 0=hard bands 1=smooth)

Beam Effects
- `/bc/beam/fx` (int) : 0=None 1=Prisma
- `/bc/invertX` (bool) persistent mirror toggle
- `/bc/hold/invertX` (bool) momentary mirror while held
- `/bc/hold/whiteFlash` (bool) force white output while held

Brightness / Scan / Dotting
- `/bc/brightness` (float 0..1 masterBrightness)
- `/bc/dot/amount` (float 0..1 density)
- `/bc/scan/pps` (int) set global scanRateHz target (points per second)
- `/bc/blackout` (bool) global output enable (true=blackout)

Flash / Strobe (momentary style)
- `/bc/flash/start` (no args or bool 1) begin full-bright flash
- `/bc/flash/release` (int ms) start decay phase (optional argument overrides default `flashReleaseMs`)

Cue System
- `/bc/cue/apply` (int cueIndex 1..30) recall stored cue
- `/bc/cue/save` (int cueIndex 1..30) store current state
- `/bc/cue/momentary` (int cueIndex) press: activate; release: send value -1 or separate `/bc/cue/momentary/release` (implementation dependent)
- `/bc/cue/learn` (int cueIndex) arm mapping for next incoming MIDI note -> momentary cue
- `/bc/cue/learn/momentary` (int cueIndex) explicit momentary learn

MIDI Mapping Helpers
- `/bc/midi/learn/cue` (int cueIndex) alias of `/bc/cue/learn`
- `/bc/midi/learn/momentary` (int cueIndex) alias

Joystick (if compiled with ofxJoystick)
- `/bc/joystick/map/momentary` (int joystickIndex, int button, int cueIndex)
- `/bc/joystick/map/save` (no args) persist joystick mappings

State Queries (reply via OSC to sender) [planned]
- `/bc/query/state` -> emits consolidated state bundle
- `/bc/query/cue` (int cueIndex) -> emits serialized cue

Legacy / Aliases (may be accepted for compatibility)
- `/beam/...` equivalent to `/bc/...`

### Value Ranges & Notes
- Position & scale normalized inputs are smoothed internally; large jumps are eased to reduce visible popping.
- Rotation, movement and wave speeds are continuous and can be negative (direction reversal).
- Setting scan PPS ramps smoothly to target to avoid DAC underruns.
- When `/bc/brightness` is driven to zero during an active flash decay, decay logic will clamp at zero and stop.

### Error Handling
Invalid indices / enums are ignored (no crash) and may log a warning to stdout.

(If an endpoint listed here appears unimplemented in your build, it may be pending integration; consult source `ofApp.cpp`.)

---
## MIDI & Controller Support
BeamCommander includes a MIDI->OSC mapper for rapid cue and parameter binding.

Tested / Intended Devices:
- Akai APC40 mk2
- Native Instruments Maschine Micro
- Any generic MIDI device (notes routed through wildcard "*" port open)

Features:
- Momentary cue mapping (press = activate, release = restore previous)
- Learning mode: arm a cue learn, press controller pad to bind
- Raw MIDI notes converted into OSC messages / internal triggers

Mappings persist to disk (JSON) alongside other app state.

---
## Joystick Support
If `ofxJoystick` is available at compile time (`HAVE_OFXJOYSTICK`), joystick buttons can be mapped to cues (momentary or latching) and forwarded via OSC. Multiple joysticks are polled; mappings persist between runs.

---
## Configuration & Persistence
Stored artifacts (created on first save/use):
- `cues.json` – saved cue states (shapes, colors, motion & effect parameters)
- Laser / scanner presets & calibration (handled internally by ofxLaser UI; stored under the app's working directory config files)
- MIDI & joystick mapping JSON files

---
## Render / Performance Notes
- PPS (points-per-second) can be globally overriden; smooth slew logic prevents sudden jumps that could underflow DAC buffers.
- Multiple movement modes (wave, random, rotation) integrate phases smoothly to avoid jitter when changing speeds.
- Rainbow color cycling and beam effects are computed per frame in `ofApp::update()` before sending to the laser manager.

---
## Safety Checklist (Non-Exhaustive)
- Physical e-stop connected and reachable
- Audience scanning compliance verified (if applicable) or disabled
- Beam path cleared of reflective / specular surfaces
- Correct optical attenuation / power for environment
- Laser output tested with low power before full brightness

---
## Development
Open the project inside openFrameworks as a standard app. Dependencies are already present in `openframeworks/addons/`.

Key source files:
- `src/ofApp.h / ofApp.cpp` – main application logic
- `src/AppState.h` – shared atomic state for UI / OSC / MIDI
- `src/MidiToOscMapper.h` – controller mapping and learn system

---
## Contributing
Pull requests and issue reports welcome (code style: follow openFrameworks conventions). Please include: platform, macOS version, hardware DAC model, reproduction steps for bugs.

---
## Acknowledgements
- openFrameworks community
- ofxLaser by Seb Lee-Delisle
- open-stage-control by Jean-Emmanuel (external dependency)
- Contributors of bundled addons (MIDI, OSC, Joystick, ImGui wrappers)

---
## Disclaimer
This software is provided "as is" without any warranty. Use at your own risk, especially when interfacing with high‑power laser hardware.

---
## Dependencies & License Summary
| Component | Role | License |
|----------|------|---------|
| BeamCommander | Core app / OSC laser control | AGPL-3.0-or-later |
| openFrameworks | Framework / runtime | MIT |
| ofxLaser | Laser DAC abstraction & rendering | MPL-2.0 |
| ofxMidi | MIDI I/O | BSD-3-Clause |
| ofxOsc (core addon) | OSC networking | MIT |
| ofxOpenCv, ofxPoco, ofxNetwork | Core addons | MIT / respective upstream |
| ofxJoystick (optional) | Joystick input | (See upstream repo) |
| open-stage-control (external) | Web OSC UI | GPL-3.0 |

See `NOTICE` for attribution; consult upstream projects for full license texts.
