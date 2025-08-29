OSC controls (example_HelloLaser)

- Receiver: UDP port 9000

Core laser controls
- /laser/shape <string>
  - Accepts: line | circle | triangle | square | wave | staticwave

- /laser/color <string|rgb>
  - Named color: "blue" | "red" | "green" (disables custom)
  - Or numeric: r g b as floats [0..1] or bytes [0..255] (enables custom)
  - Choosing any static color (named or RGB) disables rainbow automation (sets /laser/rainbow/amount=0 and /laser/rainbow/speed=0)

- /laser/wave/frequency <float>
  - Min 0.1, higher = more cycles across width

- /laser/wave/amplitude <float>
  - 0..1, fraction of half-height

- /laser/wave/speed <float>
  - Rotations/sec for the wave phase (can be 0)

- /laser/brightness <float|int>
  - Master brightness. Float [0..1] or byte [0..255].
  - Alias: /laser/master/brightness

- /laser/rotation/speed <float>
  - Rotations/sec. Negative = reverse, 0 = static

- /laser/shape/scale <float>
  - Normalized [-1..1], mapped to geometric scale factor internally

- /laser/position <x> <y>
  - Both in [-1..+1]. Also available as:
  - /laser/position/x <x>
  - /laser/position/y <y>

- /laser/dotted <float|int>
  - 0 = no dots (nothing drawn), 1 = solid (no dotting). 0..255 accepted and normalized.

Scan rate and flicker (visual gating)
- /laser/scanrate <hz>
- /laser/flicker <hz>
  - Visual flicker: gates brightness at the given Hz (50% duty). 0 disables.
  - These two addresses are aliases in this build and both control the same flicker effect.

Movement
- Rainbow (spatial color)
- /laser/rainbow/amount <0..1>
  - Spatial size: 0 = many cycles (short color segments), 1 = whole shape one color
- /laser/rainbow/speed <-1..1 or 0..255>
  - Bi-directional speed: 0 = stopped, right = positive, left = negative (maps to ~±2 cps)
- /laser/rainbow/blend <0..1>
  - Band hardness: 0 = hard steps (few bands), 1 = smooth gradient
  - Note: selecting a static color via /laser/color disables rainbow (amount and speed set to 0)

- /move/mode <string>
  - Accepts: none|off | circle | pan | tilt | eight (aliases: figure8, 8) | random
- /move/size <float|int>
  - 0..1 (or 0..255), amplitude of movement relative to canvas (0 = no movement)
- /move/speed <float>
  - Cycles per second, negative values reverse direction
  - Tip: when selecting a movement mode, if size/speed were ~0, gentle defaults are applied so motion starts immediately

Flash
- /flash <0|1>
  - Direct control used by the UI Flash button: 1=press, 0=release.
  - While flash is active (press held): master brightness is forced to full.
  - On release: if /flash/release_ms > 0, brightness fades to 0 over the specified time; else it returns instantly to the previous level.
- /flash/release_ms <int>
  - Sets flash release time in milliseconds (0..60000). 0 = instant.

MIDI mapping
- Removed in this build.

Cue system
- /cue/save
  - Arms saving: the next /cue/<n> stores a snapshot to slot n and writes to cues.json

- /cue/<n>
  - If save is armed: snapshot current state to cue n (1..16)
  - Else: recall cue n

Saved/Loaded parameters
- All laser parameters are persisted with cues except:
  - Master brightness (/laser/brightness)
  - Flash release time (/flash/release_ms)
  - Flash button state (/flash)
  
Persisted parameters include: shape, fixed/custom color (and RGB), movement (mode/size/speed), wave (freq/amp/speed), rainbow (size/speed/blend), rotation speed, shape scale, position (x/y), dotted amount, scanrate/flicker.

Notes
- The Open Stage Control panel in this repo already targets these addresses.
  - The Flash button uses /flash and a Release ms knob sends /flash/release_ms.
