# BeamCommander

BeamCommander is a professional laser show control and visualization application built on openFrameworks and ofxLaser. It unifies multi‑DAC laser output management, real‑time geometric/effect generation, a configurable OSC control surface, and hardware controller (MIDI / joystick) integration in one cohesive workflow.

## Key Capabilities
- Real‑time 2D & 3D laser shape generation, compositing, masking, and color/effect pipelines
- Multi‑DAC management (Ether Dream, Helios, LaserDock/LaserCube USB; extensible) with per‑zone calibration
- High‑performance rendering with point-rate (PPS) management and smoothing
- Integrated GUI (Dear ImGui) for configuration, calibration, preview, and live adjustment
- OSC server with a comprehensive address space for remote or automated control
- External web/touch UI via open-stage-control (not bundled)
- MIDI (e.g. Akai APC40 mk2, NI Maschine Micro) and joystick cue/parameter mapping
- Persistent cues, parameter states, and controller mappings

> SAFETY: Laser systems can be hazardous. Always observe proper laser safety practices, comply with regulations, use an emergency stop, and validate optical paths before full‑power operation.

## Licensing
Core application: **AGPL-3.0-or-later** (see `LICENSE`). Third‑party components retain their original licenses (`NOTICE`, `THIRD_PARTY_LICENSES`). Networked or hosted operation must provide complete corresponding source (AGPL requirement).

## Supported Laser DACs (via ofxLaser)
**Current**: Ether Dream (network), Helios (USB), LaserDock / LaserCube (USB).

**In Progress / Extensible**: IDN protocol integration, additional network LaserCube variants.

All detected lasers can be assigned to zones with independent transforms, color balance, scan tuning, masking, and safety adjustments.

## Platform Support
Primary target: macOS. The codebase follows openFrameworks conventions and can be rebuilt for Linux/Windows.

## Start Scripts (macOS)
Root Application:
```
./start_macOS.sh [--build]
```
Optional `--build` triggers a rebuild before launching.

External Web Control Surface (requires `/Applications/open-stage-control.app`):
```
cd third-party/open-stage-control
./start_macOS.sh [open-stage-control args]
```
Automatically loads (if present): `open-stage-control-server.config`, `open-stage-control-session.json`.

## Launch Workflow
1. **Build** (first time / after changes) – standard oF make or Xcode; verify `openframeworks/apps/myApps/BeamCommander/bin/BeamCommander.app`.
2. **Run application**:
```
./start_macOS.sh
```
3. **(Optional) Web control surface**:
```
cd third-party/open-stage-control && ./start_macOS.sh
```
Ensure OSC port alignment (BeamCommander default listen 9000).

## OSC Interface
Default UDP listen port: **9000**.

**Conventions**: Floats normalized (0..1) unless noted. Booleans 0/1. Positions -1..1. Speeds signed.

**Prefixes**: Primary `/bc`; legacy alias `/beam` accepted.

### Shape & Color
| Address | Type | Description |
|---------|------|-------------|
| /bc/shape | int | 0=Circle 1=Line 2=Triangle 3=Square 4=StaticWave |
| /bc/color | int | 0=Blue 1=Red 2=Green |
| /bc/use_custom | bool | Enable custom RGB |
| /bc/custom_rgb | float,float,float | r,g,b 0..1 |

### Geometry & Motion
| Address | Type | Description |
|---------|------|-------------|
| /bc/pos | float,float | (x,y) -1..1 |
| /bc/pos/x | float | X only |
| /bc/pos/y | float | Y only |
| /bc/scale | float | Normalized size |
| /bc/rotation/speed | float | Rotations per second |
| /bc/move/mode | int | 0=None 1=Circle 2=Pan 3=Tilt 4=Eight 5=Random |
| /bc/move/speed | float | Movement rate |
| /bc/move/size | float | Path amplitude |
| /bc/motion/hold | bool | Freeze dynamic phases |

### Wave Modulation
| /bc/wave/frequency | float | Oscillation frequency |
| /bc/wave/amplitude | float | Amplitude |
| /bc/wave/speed | float | Phase scroll speed |

### Rainbow / Color FX
| /bc/rainbow/speed | float | Cycle rate |
| /bc/rainbow/amount | float | Blend factor |
| /bc/rainbow/blend | float | 0=banded 1=smooth |

### Beam / Output Effects
| /bc/beam/fx | int | 0=None 1=Prisma |
| /bc/invertX | bool | Persistent mirror |
| /bc/hold/invertX | bool | Momentary mirror |
| /bc/hold/whiteFlash | bool | Momentary white override |

### Brightness / Scan / Dotting
| /bc/brightness | float | Master brightness |
| /bc/dot/amount | float | Dot density |
| /bc/scan/pps | int | Target points per second |
| /bc/blackout | bool | Global output disable (true=blackout) |

### Flash
| /bc/flash/start | (none/bool) | Initiate flash |
| /bc/flash/release | int(ms) | Begin decay (override default) |

### Cues
| /bc/cue/apply | int | Recall cue |
| /bc/cue/save | int | Store current state |
| /bc/cue/momentary | int | Momentary (press/hold) cue |
| /bc/cue/learn | int | Arm next MIDI note -> cue |
| /bc/cue/learn/momentary | int | Arm for momentary cue |

### MIDI Mapping Aliases
| /bc/midi/learn/cue | int | Alias of /bc/cue/learn |
| /bc/midi/learn/momentary | int | Alias of /bc/cue/learn/momentary |

### Joystick (if enabled)
| /bc/joystick/map/momentary | int,int,int | joystickIndex, button, cueIndex |
| /bc/joystick/map/save | (none) | Persist mappings |

### Queries (Planned)
| /bc/query/state | (none) | Request consolidated state |
| /bc/query/cue | int | Request cue snapshot |

**Behavior Notes**: Internal smoothing for position, scale, PPS. Invalid enums ignored (logged).

## MIDI & Controller Integration
Supported: APC40 mk2, Maschine Micro, generic MIDI (wildcard). Features: fast learn, momentary & latching cues, persistent JSON mappings.

## Joystick Integration
Active when compiled with ofxJoystick: multi-device polling, cue mapping, persistence.

## Persistence
- `cues.json` (cues)
- Mapping JSON files
- ofxLaser calibration / zone files

## Performance & Rendering
- PPS ramping to avoid DAC underruns
- Phase‑continuous motion/wave transitions
- Efficient point ordering & effect layering

## Development
Key code: `src/` (ofApp, AppState, MidiToOscMapper). Addons in `openframeworks/addons/`. Web UI configs in `third-party/open-stage-control/`.

## Contributing
Provide platform, macOS version, DAC model, steps, logs/OSC traces in issues/PRs.

## Dependency & License Summary
| Component | Role | License |
|-----------|------|---------|
| BeamCommander | Application / control layer | AGPL-3.0-or-later |
| openFrameworks | Core framework | MIT |
| ofxLaser | Laser abstraction & rendering | MPL-2.0 |
| ofxMidi | MIDI I/O | BSD-3-Clause |
| ofxOsc / core addons | Networking / utils | MIT |
| ofxJoystick (optional) | Joystick input | See upstream |
| open-stage-control (external) | Web OSC UI | GPL-3.0 |

Refer to `NOTICE` and upstream LICENSE files for full terms.

## Acknowledgements
openFrameworks community, authors of ofxLaser, and all upstream maintainers.

## Disclaimer
Provided “as is” without warranties. User assumes all responsibility, especially regarding laser hardware safety and regulatory compliance.
