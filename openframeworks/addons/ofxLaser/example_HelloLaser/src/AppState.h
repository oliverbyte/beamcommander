#pragma once

#include "ofMain.h"
#include <atomic>

struct AppState {
    enum class Shape { Circle, Line, Triangle, Square, StaticWave };
    enum class ColorSel { Blue, Red, Green };
    enum class Movement { None, Circle, Pan, Tilt, Eight, Random };
    // Beam FX selection (mutually exclusive, similar to Movement select buttons)
    enum class BeamFx { None, Prisma };

    std::atomic<Shape> currentShape{Shape::Circle};
    std::atomic<ColorSel> currentColor{ColorSel::Blue};
    std::atomic<Movement> movement{Movement::None};

    // When true, use customRGB for output color (overrides currentColor)
    std::atomic<bool> useCustomColor{false};
    // Custom color components in normalized range [0..1]
    std::atomic<float> customR{0.0f};
    std::atomic<float> customG{0.20f};
    std::atomic<float> customB{1.0f};

    // Optional params for wave
    std::atomic<float> waveFrequency{1.0f}; // cycles across width
    std::atomic<float> waveAmplitude{0.45f}; // fraction of half-height
    std::atomic<float> waveSpeed{0.0f}; // cycles per second (phase speed)
    
    // Rainbow color cycling (applied as blend on top of selected color)
    // rainbowSpeed: hue cycles per second (0 = static / disabled)
    // rainbowAmount: blend amount [0..1] where 0 = no rainbow, 1 = full rainbow
    std::atomic<float> rainbowSpeed{0.0f};
    std::atomic<float> rainbowAmount{0.0f};
    // Rainbow spatial blend (0 = hard bands, 1 = smooth gradient)
    std::atomic<float> rainbowBlend{1.0f};
    
    // Movement controls (applied additively on top of base position)
    // moveSpeed: cycles per second of the movement LFO
    // moveSize: normalized amplitude [0..1] in position space
    // Movement defaults: initialize so selecting a move FX immediately visibly animates.
    // moveSize range 0..1 -> use 50%.
    std::atomic<float> moveSpeed{0.30f}; // 5% of current max (6.0) = 0.30 cycles/sec
    std::atomic<float> moveSize{0.50f};  // 50% amplitude
    
    // Rotation (rotations per second). 0 = static.
    std::atomic<float> rotationSpeed{0.0f};
    // Smoothed target for rotation speed (knob input) to hide MIDI stepping.
    std::atomic<float> rotationSpeedTarget{0.0f};

    // Motion hold: when true, temporarily pauses rotation and movement (LFO & wave) without losing original speeds.
    // Implementation detail: we cache pre-hold speeds the moment hold is engaged; releasing restores them.
    std::atomic<bool> motionHold{false};
    std::atomic<float> heldRotationSpeed{0.0f};
    std::atomic<float> heldMoveSpeed{0.0f};
    std::atomic<float> heldWaveSpeed{0.0f};

    // Normalized shape scale control in [-1..1]. 0.0 = unscaled (100%).
    // Mapped in draw() to geometric factor s in [-3..3] via a quadratic fit.
    std::atomic<float> shapeScale{0.0f};
    std::atomic<float> shapeScaleTarget{0.0f}; // smoothing target for scale

    // Normalized position offsets. Range [-1..+1] each.
    // Mapped to [-500%, +500%] of half-dimension in draw (i.e., up to ±2.5x canvas size).
    std::atomic<float> posNormX{0.0f};
    std::atomic<float> posNormY{0.0f};
    // Smoothing targets for manual position (to hide MIDI 7‑bit stepping).
    // OSC /laser/position*, when received, writes to these targets; per-frame we slew current pos toward target.
    std::atomic<float> posTargetX{0.0f};
    std::atomic<float> posTargetY{0.0f};

    // Axis inversion flags
    std::atomic<bool> invertX{false};
    // Momentary hold inversion (set on note ON, cleared on note OFF for a specific key)
    std::atomic<bool> holdInvertX{false};

    // Momentary white flash override: when true, force output color to pure white regardless of other color settings.
    std::atomic<bool> holdWhiteFlash{false};

    // Blackout: when true, suppress DAC output but still render preview.
    std::atomic<bool> blackout{false};

    // Selected beam FX. Extend this enum as more effects are added.
    std::atomic<BeamFx> beamFx{BeamFx::None};

    // Global master brightness multiplier [0..1]. Applied to final RGB in draw.
    std::atomic<float> masterBrightness{1.0f};

    // Dotted effect amount [0..1].
    // 0 = no drawing (0 dots), 1 = normal solid shapes (no dotting). Values in between create dots/dashes.
    std::atomic<float> dotAmount{1.0f};
    // Smoothing target for dotted effect.
    std::atomic<float> dotAmountTarget{1.0f};

    // (scan rate / flicker feature removed)
    // Scanrate now used as strobe frequency (Hz). 0 = no strobe.
    std::atomic<float> scanRateHz{0.0f};

    ofColor toOfColor(float rainbowHue01 = -1.0f) const {
        if (useCustomColor.load()) {
            const float r = ofClamp(customR.load(), 0.0f, 1.0f);
            const float g = ofClamp(customG.load(), 0.0f, 1.0f);
            const float b = ofClamp(customB.load(), 0.0f, 1.0f);
            ofColor base(static_cast<unsigned char>(r * 255.0f),
                         static_cast<unsigned char>(g * 255.0f),
                         static_cast<unsigned char>(b * 255.0f));
            if(rainbowHue01 >= 0.0f){
                float amt = ofClamp(rainbowAmount.load(), 0.0f, 1.0f);
                if(amt > 0.0f){
                    ofColor rb; rb.setHsb((unsigned char)ofClamp(rainbowHue01*255.0f, 0.0f, 255.0f), 255, 255);
                    base = base.lerp(rb, amt);
                }
            }
            return base;
        } else {
            ofColor base;
            switch (currentColor.load()) {
                case ColorSel::Red: base = ofColor(255, 0, 20); break;
                case ColorSel::Green: base = ofColor(0, 220, 80); break;
                case ColorSel::Blue:
                default: base = ofColor(0, 50, 255); break;
            }
            if(rainbowHue01 >= 0.0f){
                float amt = ofClamp(rainbowAmount.load(), 0.0f, 1.0f);
                if(amt > 0.0f){
                    ofColor rb; rb.setHsb((unsigned char)ofClamp(rainbowHue01*255.0f, 0.0f, 255.0f), 255, 255);
                    base = base.lerp(rb, amt);
                }
            }
            return base;
        }
    }
};
