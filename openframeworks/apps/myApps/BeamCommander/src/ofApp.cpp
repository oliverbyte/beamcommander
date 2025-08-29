// Full original implementation migrated from example_HelloLaser (unabridged)
#include "ofApp.h"
#include "ofAppGLFWWindow.h"
#include <GLFW/glfw3.h>
#include "ofxLaserUI.h"



//--------------------------------------------------------------
void ofApp::setup(){
    
	// NOTE that you no longer need to set up anything at all in code!
	// If you want to change the size of the laser area, use
	// laser.setCanvasSize(int width, int height) - default is 800 x 800.
	ofSetVerticalSync(false); 
	// no star poly needed
	osc.setup(oscPort);
	// Start MIDI->OSC mapper (opens port matching "APC" if present)
	midiMapper = std::make_unique<MidiToOscMapper>();
	// Open all MIDI ports so both APC and Maschine (or other controllers) are captured
	midiMapper->setup("*", oscPort);
	// Install raw note callback for learn feature
	midiMapper->onRawNote = [this](int ch, int note){
		// Standard (non-momentary) cue learn
		if(learnArmed.load() && learnCueIndex.load() > 0){
			int cueIdx = learnCueIndex.load();
			// Always create momentary mapping now
			midiMapper->addOrUpdateMomentaryCueNoteMapping(ch, note, cueIdx);
			ofLogNotice() << "Learn: mapped MIDI ch " << ch << " note " << note << " -> momentary cue " << cueIdx;
			learnArmed.store(false);
			learnCueIndex.store(0);
		}
		// Momentary cue learn
		if(learnMomentaryArmed.load() && learnMomentaryCueIndex.load() > 0){
			int cueIdx = learnMomentaryCueIndex.load();
			midiMapper->addOrUpdateMomentaryCueNoteMapping(ch, note, cueIdx);
			ofLogNotice() << "Learn: mapped MIDI ch " << ch << " note " << note << " -> momentary cue " << cueIdx;
			learnMomentaryArmed.store(false);
			learnMomentaryCueIndex.store(0);
		}
	};
	// Joystick mapping persistence & sender
	joystickOscSender.setup("127.0.0.1", oscPort);
	loadJoystickCueMappings();
	// Load persisted cues from disk, if present
	loadCuesFromDisk();
	// Sanity clamp any persisted rotation speed. Expanded to [-400,400] to allow very high requested speeds.
	{
		float rs = state->rotationSpeed.load();
		if(rs < -400.0f || rs > 400.0f){
			float clamped = ofClamp(rs, -400.0f, 400.0f);
			state->rotationSpeed.store(clamped);
			ofLogNotice() << "Clamped persisted rotationSpeed " << rs << " -> " << clamped;
		}
	}

	// Apply default global PPS override at startup if none specified yet.
	// Requirement: if nothing set on app startup, use maximum (20000).
	if(state->scanRateHz.load() <= 0.0f){
		const int defaultPPS = 20000;
		ppsTarget = defaultPPS;
		ppsCurrent = defaultPPS; // baseline; apply immediately at startup
		int num = laser.getNumLasers();
		for(int i=0;i<num;++i){
			try { laser.getLaser(i).setPpsOverride(ppsCurrent); } catch(...) {}
		}
		state->scanRateHz.store((float)ppsCurrent); // store applied value (not just target)
		ofLogNotice() << "Startup: applying default PPS override " << ppsCurrent;
	} else {
		// If persisted value exists, treat it as target and current to avoid ramp spike.
		ppsTarget = (int)state->scanRateHz.load();
		ppsCurrent = ppsTarget;
	}

	// Initialize per-run seeds for Random movement to avoid repeating paths
	randomSeedX = ofRandom(1.0f, 1000.0f);
	randomSeedY = ofRandom(1001.0f, 2000.0f);
	// Ensure position smoothing targets start aligned with current (avoids initial drift)
	state->posTargetX.store(state->posNormX.load());
	state->posTargetY.store(state->posNormY.load());
	state->shapeScaleTarget.store(state->shapeScale.load());
	state->rotationSpeedTarget.store(state->rotationSpeed.load());
	state->dotAmountTarget.store(state->dotAmount.load());
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
	// prepares laser manager to receive new graphics
	laser.update();

	// Smoothly ramp PPS toward target to avoid large instantaneous jumps that can
	// cause DAC buffer underruns (red indicator). Only if enabled (target>0).
	if(ppsTarget != ppsCurrent){
		// Time-based slew (deltaTime seconds) but capped by legacy per-frame limit.
		float dt = ofGetLastFrameTime();
		if(dt <= 0.f) dt = 1.f/60.f; // fallback
		int maxStepTime = (int)std::ceil(ppsSlewPerSecond * dt);
		int maxStep = std::min(maxStepTime, ppsSlewPerFrame); // additionally cap by old frame limit
		if(maxStep < 1) maxStep = 1;
		int delta = ppsTarget - ppsCurrent;

		if(ppsCurrent == 0 && ppsTarget > 0){
			// Enable sequence: seed to minPPS (or target if lower) then let ramp proceed next frames.
			const int minPPS = 2000; // lowered from 3000 to allow slower safe scan rate
			int seed = (ppsTarget < minPPS) ? ppsTarget : minPPS;
			if(seed != ppsCurrent){
				ppsCurrent = seed;
				ofLogNotice() << "PPS enable seed -> " << ppsCurrent << " (target " << ppsTarget << ")";
			}
		} else {
			int step = ofClamp(delta, -maxStep, maxStep);
			ppsCurrent += step;
			// Avoid oscillatory overshoot; snap when within one step.
			if(std::abs(ppsTarget - ppsCurrent) <= maxStep){
				ppsCurrent = ppsTarget;
			}
		}
		if(ppsCurrent < 0) ppsCurrent = 0; // safety
		int num = laser.getNumLasers();
		for(int i=0;i<num;++i){
			try { laser.getLaser(i).setPpsOverride(ppsCurrent); } catch(...) {}
		}
		state->scanRateHz.store((float)ppsCurrent); // reflect applied value
	}

	// Motion hold: if engaged, we freeze phases and zero effective speeds (but do not reset angles)
	bool hold = state->motionHold.load();
	float rotSpeed = state->rotationSpeed.load();
	if(hold){
		rotSpeed = 0.0f; // effective speed
	}
	// Integrate rotation cumulatively so changing speed doesn't reset orientation
	if (rotSpeed == 0.0f) {
		// Do NOT reset orientation when paused; only reset when actual speed param is 0 and not holding
	} else {
		rotationAngleRad += TWO_PI * rotSpeed * ofGetLastFrameTime();
		rotationAngleRad = fmodf(rotationAngleRad, TWO_PI);
		if (rotationAngleRad < 0.0f) rotationAngleRad += TWO_PI;
	}

	// Integrate wave phase cumulatively so changing speed does not cause phase jumps
	{
	float speed = state->waveSpeed.load(); // cycles per second; can be negative
	if(hold) speed = 0.0f; // pause
	if (speed == 0.0f) {
			// Hold current phase; do not reset to preserve continuity when resuming
		} else {
			wavePhaseRad += TWO_PI * speed * ofGetLastFrameTime();
			// Wrap to [0, 2*pi)
			wavePhaseRad = fmodf(wavePhaseRad, TWO_PI);
			if (wavePhaseRad < 0.0f) wavePhaseRad += TWO_PI;
		}
	}

	// Integrate movement phase cumulatively so changing speed does not cause jumps
	{
		float ms = state->moveSpeed.load(); // cycles per second; can be negative
		if(hold) ms = 0.0f; // pause
		if (ms == 0.0f) {
			// Hold current phase
		} else {
			movePhaseRad += TWO_PI * ms * ofGetLastFrameTime();
			movePhaseRad = fmodf(movePhaseRad, TWO_PI);
			if (movePhaseRad < 0.0f) movePhaseRad += TWO_PI;
		}
	// For Random movement, also advance an unbounded time accumulator in cycles (no wrapping)
	if(!hold){
		moveTimeCycles += static_cast<double>(ms) * static_cast<double>(ofGetLastFrameTime());
	}
	}

	updateOsc();

	// After processing any new OSC/MIDI messages this frame, smoothly slew current manual position
	// and scale toward targets. Adaptive exponential smoothing: ultra-fine blending for tiny knob moves
	// while keeping responsiveness for large gestures.
	{
		float dt = ofGetLastFrameTime();
		if(dt > 0.25f) dt = 0.25f; // clamp hitches
		if(dt < 0.f) dt = 0.f;

		auto smoothOne = [&](float cur, float target, float tauBase){
			float baseAlpha = 1.0f - expf(-dt / std::max(0.0001f, tauBase));
			float dist = fabsf(target - cur);
			// Micro movement (slow knob) refinement: scale alpha down for <0.02 normalized travel
			if(dist < 0.02f){
				float slowFactor = ofMap(dist, 0.0f, 0.02f, 0.15f, 1.0f, true);
				baseAlpha *= slowFactor;
			}
			// Large jump acceleration
			if(dist > 0.25f){
				float boost = ofMap(ofClamp(dist, 0.25f, 2.0f), 0.25f, 2.0f, 1.0f, 3.0f, true);
				baseAlpha = 1.0f - powf(1.0f - baseAlpha, boost);
			}
			return cur + (target - cur) * baseAlpha;
		};

		float curX = state->posNormX.load();
		float curY = state->posNormY.load();
		float tgtX = ofClamp(state->posTargetX.load(), -1.0f, 1.0f);
		float tgtY = ofClamp(state->posTargetY.load(), -1.0f, 1.0f);
	float curScale = state->shapeScale.load();
	float tgtScale = ofClamp(state->shapeScaleTarget.load(), -1.0f, 1.0f);
	float curRot = state->rotationSpeed.load();
	float tgtRot = state->rotationSpeedTarget.load();
	float curDots = state->dotAmount.load();
	float tgtDots = ofClamp(state->dotAmountTarget.load(), 0.0f, 1.0f);

		curX = smoothOne(curX, tgtX, 0.090f);
		curY = smoothOne(curY, tgtY, 0.090f);
	curScale = smoothOne(curScale, tgtScale, 0.120f);
	curRot = smoothOne(curRot, tgtRot, 0.150f);    // slightly slower smoothing for rotation
	curDots = smoothOne(curDots, tgtDots, 0.100f); // responsive yet smooth

		state->posNormX.store(curX);
		state->posNormY.store(curY);
	state->shapeScale.store(curScale);
	state->rotationSpeed.store(curRot);
	state->dotAmount.store(curDots);
	}
}

// MIDI removed


void ofApp::draw() {
	ofBackground(5, 5, 10);

	// Draw selected shape maximized on default 800x800 canvas
	const float W = 800;
	const float H = 800;

	// Update rainbow phase based on speed (cycles per second)
	{
		const float rs = state->rainbowSpeed.load();
		if (rs != 0.0f) {
			rainbowPhaseRad += rs * TWO_PI * ofGetLastFrameTime();
			while (rainbowPhaseRad >= TWO_PI) rainbowPhaseRad -= TWO_PI;
			while (rainbowPhaseRad < 0.0f) rainbowPhaseRad += TWO_PI;
		}
	}

	// Poll joystick buttons for learn mappings and runtime triggers
	pollJoysticksForLearningAndTriggers();
	// Spatial rainbow: if Rainbow Size > 0, distribute colors left->right across the canvas.
	// Rainbow Speed still animates the gradient over time as a phase offset.
	const bool whiteFlash = state->holdWhiteFlash.load();
	const float spatialSize = ofClamp(state->rainbowAmount.load(), 0.0f, 1.0f);
	const bool spatialRainbow = (!whiteFlash) && (spatialSize > 0.0f);
	const float hueBlendInput = (!whiteFlash && !spatialRainbow && state->rainbowSpeed.load() != 0.0f)
									? (rainbowPhaseRad / TWO_PI)
									: -1.0f;
	ofColor col = whiteFlash ? ofColor(255,255,255)
							  : state->toOfColor(hueBlendInput);
	// Apply master brightness (0..1). Accepts 0..255 in OSC handler; stored here normalized.
	float mb = ofClamp(state->masterBrightness.load(), 0.0f, 1.0f);
	if (flashActive) {
		// While flashing, force to full and cancel any decay in progress
		mb = 1.0f;
		flashDecaying = false;
	} else if (flashDecaying) {
		// Apply decay curve from flashDecayFrom to 0 over flashReleaseMs
		const int durMs = std::max(0, flashReleaseMs.load());
		const uint64_t now = ofGetElapsedTimeMillis();
		const uint64_t elapsed = (now > flashDecayStartMs) ? (now - flashDecayStartMs) : 0;
		if (durMs <= 0 || elapsed >= (uint64_t)durMs) {
			mb = 0.0f;
			flashDecaying = false;
		} else {
			float t = (float)elapsed / (float)durMs; // 0..1
			// Linear ramp for now; could be eased if desired
			float target = ofLerp(flashDecayFrom, 0.0f, t);
			mb = target;
		}
		// Write back clamped value to state so UI reflects the decay
		state->masterBrightness.store(ofClamp(mb, 0.0f, 1.0f));
	}
	// (strobe removed: brightness no longer gated by scanrate)
	col.r = static_cast<unsigned char>(col.r * mb);
	col.g = static_cast<unsigned char>(col.g * mb);
	col.b = static_cast<unsigned char>(col.b * mb);

	// Helper: color as a function of X for spatial rainbow
	auto colorForX = [&](float x) -> ofColor {
		if (whiteFlash || !spatialRainbow) return col; // fall back to single (possibly flash white)
		float xNorm = ofClamp(x / W, 0.0f, 1.0f);
		// More size -> fewer cycles across width -> longer same-color segments.
		// At max size, cyclesAcrossWidth=0 => whole shape one color.
		float cyclesAcrossWidth = ofLerp(24.0f, 0.0f, spatialSize);
		float hue01 = fmodf((rainbowPhaseRad / TWO_PI) + xNorm * cyclesAcrossWidth, 1.0f);
		if (hue01 < 0.0f) hue01 += 1.0f;
		// Rainbow Blend controls band hardness via hue quantization:
		// 0 = hard bands (few), 1 = smooth continuous gradient (many steps).
		float blend = ofClamp(state->rainbowBlend.load(), 0.0f, 1.0f);
		if (blend < 0.999f) {
			float stepsF = ofLerp(6.0f, 256.0f, blend * blend); // perceptual ramp
			int steps = std::max(2, (int)roundf(stepsF));
			hue01 = floorf(hue01 * steps) / (float)steps;
		}
		ofColor c; c.setHsb((unsigned char)ofClamp(hue01 * 255.0f, 0.0f, 255.0f), 255, 255);
		// Apply master brightness
		c.r = static_cast<unsigned char>(c.r * mb);
		c.g = static_cast<unsigned char>(c.g * mb);
		c.b = static_cast<unsigned char>(c.b * mb);
		return c;
	};

	// Scale control: default to unscaled (1.0) until we receive a scale input via OSC or cue.
	// Once input is received, map normalized [-1..1] to clamped positive [minScale..maxScale].
	float sFactor = 1.0f; // unscaled by default
	{
		const float minScale = 0.001f; // allow very small but still positive (no inversion)
		const float maxScale = 3.0f;   // generous upper bound
		if (hasScaleInput) {
			const float sNorm = ofClamp(state->shapeScale.load(), -1.0f, 1.0f);
			sFactor = ofMap(sNorm, -1.0f, 1.0f, minScale, maxScale, true);
		}
	}

	// Normalized position [-1..+1] mapped to [-100%, +100%] of half-dimension (±1x canvas)
	float nx = ofClamp(state->posNormX.load(), -1.0f, 1.0f);
	const float ny = ofClamp(state->posNormY.load(), -1.0f, 1.0f);
	const ofVec2f baseCenter(W * 0.5f, H * 0.5f);
	// Invert Y so +1 moves up (screen Y grows down). A small margin keeps shapes inside view.
		const float travel = 1.25f; // 1.25 of half-dimension allows moving fully off-screen
	ofVec2f delta(nx * (W * 0.5f) * travel, -ny * (H * 0.5f) * travel);

	// Apply movement offset (LFO) on top of manual position. When speed is 0 we freeze (stall) at last offset.
	{
		const auto mv = state->movement.load();
		const float mvSize = ofClamp(state->moveSize.load(), 0.0f, 1.0f);
		const float mvSpeed = state->moveSpeed.load(); // cycles/sec
		bool movingNow = (mv != AppState::Movement::None && mvSize > 0.0001f && mvSpeed != 0.0f);
		if(movingNow){
			ofVec2f curOffset(0,0);
			const float phase = movePhaseRad; // accumulated phase for continuity
			const float ax = (W * 0.5f) * 1.0f * mvSize;
			const float ay = (H * 0.5f) * 1.0f * mvSize;
			switch (mv) {
				case AppState::Movement::Circle: {
					curOffset.set(ax * cosf(phase), ay * sinf(phase)); break; }
				case AppState::Movement::Pan: {
					curOffset.set(ax * sinf(phase), 0.0f); break; }
				case AppState::Movement::Tilt: {
					curOffset.set(0.0f, ay * sinf(phase)); break; }
				case AppState::Movement::Eight: {
					curOffset.set(ax * sinf(phase), ay * sinf(2.0f * phase)); break; }
				case AppState::Movement::Random: {
					const double t = moveTimeCycles;
					float nx = ofNoise(static_cast<float>(t * 0.60), randomSeedX);
					float ny = ofNoise(static_cast<float>(t * 0.70), randomSeedY);
					float fx = nx * 2.0f - 1.0f;
					float fy = ny * 2.0f - 1.0f;
					curOffset.set(ax * fx, ay * fy); break; }
				default: break;
			}
			delta += curOffset;
			lastMoveOffset = curOffset; // remember where we are
		} else {
			// Movement disabled or speed zero: stall at last offset (freeze motion visually)
			delta += lastMoveOffset;
		}
	}

	// Use cumulative rotation angle integrated in update()
	const float angleRad = rotationAngleRad;


	const bool effectiveMirror = state->invertX.load() ^ state->holdInvertX.load();
	// Helper: Scale, Rotate about center, then Translate by movement delta; finally apply global mirror if active.
	auto transformSRD = [&](const ofVec2f &p0, const ofVec2f &c) {
		ofVec2f d = p0 - c;
		d *= sFactor; // uniform scale
		if (angleRad != 0.0f) {
			float s = sinf(angleRad), ccos = cosf(angleRad);
			d.set(d.x * ccos - d.y * s, d.x * s + d.y * ccos);
		}
		ofVec2f out = c + d + delta;
	if(effectiveMirror){
			out.x = W - out.x; // mirror whole output
		}
		return out;
	};

	auto maybeMirror = [&](ofVec2f &p){ /* no-op now; global handled in transformSRD */ };

	// Scan-safety thresholds (in pixels)
	const float kSafeMinRadius = 6.0f; // min circle radius
	const float kSafeMinLength = 12.0f; // min line length
	const float kSafeMinDim = 12.0f;    // min polygon dimension

	// Dotting control: if dottedAmount is 0.0 or 1.0, disable dotting and draw solid shapes.
	const float dottedAmt = ofClamp(state->dotAmount.load(), 0.0f, 1.0f);
	const bool disableDotting = (dottedAmt <= 0.001f || dottedAmt >= 0.999f);

	// Helper: render as dots along the path using spacing controlled by dotAmount.
	// Optionally apply a phaseOffset (in pixels of arc length along the polyline) to shift dot positions.
	// dotAmount controls spacing (density): 0 -> nothing, 1 -> solid (no gaps).
	auto drawDottedAlongPolyline = [&](const ofPolyline &polySrc, float phaseOffset) {
		float amt = ofClamp(state->dotAmount.load(), 0.0f, 1.0f);
		if (amt <= 0.0f) return; // 0 dots
		if (amt >= 0.999f) {
			if (spatialRainbow) {
				ofPolyline res = polySrc.getResampledBySpacing(6.0f);
				const auto &pts = res.getVertices();
				for (size_t i = 0; i + 1 < pts.size(); ++i) {
					const ofVec2f &a = pts[i];
					const ofVec2f &b = pts[i+1];
					ofColor cc = colorForX((a.x + b.x) * 0.5f);
					laser.drawLine(a.x, a.y, b.x, b.y, cc, OFXLASER_PROFILE_FAST);
				}
			} else {
				laser.drawPoly(polySrc, col, OFXLASER_PROFILE_FAST);
			}
			return;
		}

		if (polySrc.size() < 2) return;

		// Ensure we have a copy with explicit closed state preserved
		ofPolyline poly = polySrc;
		const bool closed = poly.isClosed();
		const float perim = poly.getPerimeter();
		if (perim <= 1.0f) return;

		// Map amt to spacing: small amt -> large spacing (few dots), large amt -> small spacing (dense)
		const float minSpacingPx = 2.0f;   // near-solid when dotAmount ~ 1
		const float maxSpacingPx = 120.0f; // sparse when dotAmount ~ 0
		const float spacing = ofMap(amt, 0.0f, 1.0f, maxSpacingPx, minSpacingPx, true);

		// Emit single-pixel dots along the path.
		auto drawDotAt = [&](float lengthAlong) {
			float L = lengthAlong;
			if (closed) {
				// wrap length for closed paths
				L = fmodf(L, perim);
				if (L < 0) L += perim;
			} else {
				// clamp for open paths
				L = ofClamp(L, 0.0f, perim);
			}
			ofVec2f p = poly.getPointAtLength(L);
			ofColor cc = colorForX(p.x);
			laser.drawDot(p.x, p.y, cc, 1.0f, OFXLASER_PROFILE_FAST);
		};

		// Start from phaseOffset so rotation can shift dot placement for closed shapes (e.g., circles)
		for (float center = phaseOffset; center < perim + phaseOffset; center += spacing) {
			drawDotAt(center);
		}
	};

	auto drawDottedLine = [&](const ofVec2f &a, const ofVec2f &b) {
		float amt = ofClamp(state->dotAmount.load(), 0.0f, 1.0f);
		if (amt <= 0.001f || amt >= 0.999f) {
			if (spatialRainbow) {
				const int segs = 100;
				for (int i = 0; i < segs; ++i) {
					float t0 = (float)i / segs;
					float t1 = (float)(i+1) / segs;
					ofVec2f p0 = a.getInterpolated(b, t0);
					ofVec2f p1 = a.getInterpolated(b, t1);
					ofColor cc = colorForX((p0.x + p1.x) * 0.5f);
					laser.drawLine(p0.x, p0.y, p1.x, p1.y, cc, OFXLASER_PROFILE_FAST);
				}
			} else {
				laser.drawLine(a.x, a.y, b.x, b.y, col, OFXLASER_PROFILE_FAST);
			}
			return;
		}
		ofPolyline pl;
		pl.setClosed(false);
		pl.addVertex(a.x, a.y);
		pl.addVertex(b.x, b.y);
		drawDottedAlongPolyline(pl, 0.0f);
	};

	auto drawDottedCircle = [&](const ofVec2f &center, float radius) {
		float amt = ofClamp(state->dotAmount.load(), 0.0f, 1.0f);
		if (amt <= 0.001f || amt >= 0.999f) {
			if (spatialRainbow) {
				ofPolyline pl;
				pl.setClosed(true);
				const int steps = std::max(64, (int)roundf(TWO_PI * radius / 4.0f));
				for (int i = 0; i < steps; i++) {
					float a = (TWO_PI * i) / steps;
					pl.addVertex(center.x + radius * cosf(a), center.y + radius * sinf(a));
				}
				ofPolyline res = pl.getResampledBySpacing(6.0f);
				const auto &pts = res.getVertices();
				for (size_t i = 0; i + 1 < pts.size(); ++i) {
					const ofVec2f &a = pts[i];
					const ofVec2f &b = pts[i+1];
					ofColor cc = colorForX((a.x + b.x) * 0.5f);
					laser.drawLine(a.x, a.y, b.x, b.y, cc, OFXLASER_PROFILE_FAST);
				}
			} else {
				float cx = effectiveMirror ? (W - center.x) : center.x; // center-only circle draw bypasses transformSRD
				laser.drawCircle(cx, center.y, radius, col, OFXLASER_PROFILE_FAST);
			}
			return;
		}
		// Approximate circle as a polyline; resolution scales with radius
		ofPolyline pl;
		pl.setClosed(true);
		const int steps = std::max(64, (int)roundf(TWO_PI * radius / 4.0f));
		for (int i = 0; i < steps; i++) {
			float a = (TWO_PI * i) / steps;
			pl.addVertex(center.x + radius * cosf(a), center.y + radius * sinf(a));
		}
	// Phase offset for circle to reflect rotation of dotted pattern: s = r * theta
	float theta = angleRad; // radians (already wrapped)
	float phaseOffsetLen = theta * radius; // arc length offset along perimeter
	drawDottedAlongPolyline(pl, phaseOffsetLen);
	};

	// Prism effect duplication logic: determine iteration count and per-pass modifiers
	bool prismActive = (state->beamFx.load() == AppState::BeamFx::Prisma);
	int prismCopies = prismActive ? 5 : 1;
	// 5-fold circular arrangement (no central beam) similar to 5-facet prism/gobo.
	// Offsets are computed dynamically each frame to adapt to current scale.
	// Prism separation: keep relative spacing consistent by scaling offsets roughly with absolute shape scale.
	// For very small scales, allow tighter grouping; for large, expand proportionally but clamp to avoid leaving frame.
	float scaleMag = fabsf(sFactor);
	// Estimate shape radial extent (used to keep prism facets from overlapping when scaling up)
	AppState::Shape shapeNow = state->currentShape.load();
	float shapeRadiusEstimate = 0.0f;
	if(shapeNow == AppState::Shape::Circle){
		float circleBaseR = (std::min(W, H) * 0.5f - 10.0f);
		shapeRadiusEstimate = circleBaseR * scaleMag;
	} else if(shapeNow == AppState::Shape::Square || shapeNow == AppState::Shape::Triangle){
		// Approximate inscribed circle radius for other compact shapes
		shapeRadiusEstimate = (std::min(W,H) * 0.25f) * scaleMag; // heuristic
	} else if(shapeNow == AppState::Shape::Line){
		// Use half the thickness proxy (line has little radial extent); keep facets closer
		shapeRadiusEstimate = (std::min(W,H) * 0.08f) * scaleMag;
	} else {
		shapeRadiusEstimate = (std::min(W,H) * 0.2f) * scaleMag;
	}
	float sep = 0.0f; // prism separation
	float localScaleAdjust = 1.0f; // additional per-copy scale only for prism
	if(prismActive){
		// Unbounded growth: let radius and separation expand; facets can go off-canvas intentionally.
		// Keep proportional geometry: sep = k * rWanted.
		const float kSepFactor = 1.73f;
		float rWanted = shapeRadiusEstimate; // full scaled radius
		sep = kSepFactor * rWanted;
		localScaleAdjust = 1.0f; // never shrink
	}
		for(int prismIdx = 0; prismIdx < prismCopies; ++prismIdx) {
		float angle = -HALF_PI + prismIdx * TWO_PI / 5.0f; // start at top, clockwise
	ofVec2f extraOffset = prismActive ? ofVec2f(cosf(angle)*sep, sinf(angle)*sep) : ofVec2f(0,0);
		ofColor colPrism = col; // start from base color each pass
		// Optional subtle brightness modulation (disabled for uniform look) - can be re-enabled if desired
		// float mod = 0.9f + 0.1f * sinf(prismIdx * TWO_PI / prismCopies);
		// colPrism.r = (uint8_t)ofClamp(colPrism.r * mod, 0.0f, 255.0f);
		// colPrism.g = (uint8_t)ofClamp(colPrism.g * mod, 0.0f, 255.0f);
		// colPrism.b = (uint8_t)ofClamp(colPrism.b * mod, 0.0f, 255.0f);
		// Custom transform including prism offset by augmenting delta component
		auto transformSRDPrism = [&](const ofVec2f &p0, const ofVec2f &c){
			ofVec2f d = p0 - c;
			d *= (sFactor * localScaleAdjust);
			if (angleRad != 0.0f) {
				float s = sinf(angleRad), ccos = cosf(angleRad);
				d.set(d.x * ccos - d.y * s, d.x * s + d.y * ccos);
			}
			ofVec2f out = c + d + delta + extraOffset; // add original movement delta + prism offset
			if(effectiveMirror){ out.x = W - out.x; }
			return out;
		};
	switch (state->currentShape.load()) {
		case AppState::Shape::Circle: {
			const float baseR = (std::min(W, H) * 0.5f - 10.0f);
			float r = baseR * fabsf(sFactor) * (prismActive ? localScaleAdjust : 1.0f);
			// Scan-safety: avoid drawing extremely small radius which becomes a hot dot
			if (r < kSafeMinRadius) break;
			ofVec2f c = baseCenter + delta + extraOffset; // apply movement + prism offset
			if(effectiveMirror){ c.x = W - c.x; } // center needs explicit mirror because circle draw bypasses transformSRD
			if (disableDotting) {
				if (spatialRainbow) {
					ofPolyline pl;
					pl.setClosed(true);
					const int steps = std::max(64, (int)roundf(TWO_PI * r / 4.0f));
					for (int i = 0; i < steps; i++) {
						float a = (TWO_PI * i) / steps;
						float vx = c.x + r * cosf(a);
						pl.addVertex(vx, c.y + r * sinf(a));
					}
					ofPolyline res = pl.getResampledBySpacing(6.0f);
					const auto &pts = res.getVertices();
					for (size_t i = 0; i + 1 < pts.size(); ++i) {
						const ofVec2f &a = pts[i];
						const ofVec2f &b = pts[i+1];
						ofColor cc = colorForX((a.x + b.x) * 0.5f);
						laser.drawLine(a.x, a.y, b.x, b.y, cc, OFXLASER_PROFILE_FAST);
					}
				} else {
					laser.drawCircle(c.x, c.y, r, colPrism, OFXLASER_PROFILE_FAST);
				}
			} else {
				drawDottedCircle(c, r);
			}
			// (debug orientation marker removed)
			break;
		}
		case AppState::Shape::Line: {
			const float pad0 = 10.0f;
			const float baseLen = W - 2.0f * pad0;
			// Scan-safety: skip drawing if scaled length becomes too short
			if (fabsf(sFactor) * baseLen < kSafeMinLength) break;
			ofVec2f c = baseCenter;
			ofVec2f p1 = transformSRDPrism({pad0, H * 0.5f}, c); maybeMirror(p1);
			ofVec2f p2 = transformSRDPrism({W - pad0, H * 0.5f}, c); maybeMirror(p2);
		if (disableDotting) {
				if (spatialRainbow) {
					const int segs = 120;
					for (int i = 0; i < segs; ++i) {
						float t0 = (float)i / segs;
						float t1 = (float)(i + 1) / segs;
						ofVec2f a = p1.getInterpolated(p2, t0);
						ofVec2f b = p1.getInterpolated(p2, t1);
						ofColor cc = colorForX((a.x + b.x) * 0.5f);
						laser.drawLine(a.x, a.y, b.x, b.y, cc, OFXLASER_PROFILE_FAST);
					}
				} else {
			laser.drawLine(p1.x, p1.y, p2.x, p2.y, colPrism, OFXLASER_PROFILE_FAST);
				}
			} else {
				drawDottedLine(p1, p2);
			}
			// (debug line orientation marker removed)
			break;
		}
		case AppState::Shape::Triangle: {
			const float pad0 = 10.0f;
			float baseDim = W - 2.0f * pad0;
			// Scan-safety: skip if triangle would be too tiny
			if (fabsf(sFactor) * baseDim < kSafeMinDim) break;
			ofVec2f c = baseCenter;
			ofVec2f v1 = transformSRDPrism({W * 0.5f, pad0}, c); maybeMirror(v1);
			ofVec2f v2 = transformSRDPrism({W - pad0, H - pad0}, c); maybeMirror(v2);
			ofVec2f v3 = transformSRDPrism({pad0, H - pad0}, c); maybeMirror(v3);
			ofPolyline tri;
			tri.setClosed(true);
			tri.addVertex(v1.x, v1.y);
			tri.addVertex(v2.x, v2.y);
			tri.addVertex(v3.x, v3.y);
			if (disableDotting) {
				if (spatialRainbow) {
					const float resampleSpacing = 6.0f; // pixels
					ofPolyline triResampled = tri.getResampledBySpacing(resampleSpacing);
					const auto &pts = triResampled.getVertices();
					for (size_t i = 0; i + 1 < pts.size(); ++i) {
						const ofVec2f &a = pts[i];
						const ofVec2f &b = pts[i+1];
						ofColor cc = colorForX((a.x + b.x) * 0.5f);
						laser.drawLine(a.x, a.y, b.x, b.y, cc, OFXLASER_PROFILE_FAST);
					}
				} else {
					laser.drawPoly(tri, colPrism, OFXLASER_PROFILE_FAST);
				}
			} else {
				// Resample to create enough points for dotting
				const float resampleSpacing = 6.0f; // pixels
				ofPolyline triResampled = tri.getResampledBySpacing(resampleSpacing);
				drawDottedAlongPolyline(triResampled, 0.0f);
			}
			// (debug triangle orientation marker removed)
			break;
		}
		case AppState::Shape::Square: {
			const float pad0 = 10.0f;
			float baseDim = W - 2.0f * pad0;
			if (fabsf(sFactor) * baseDim < kSafeMinDim) break;
			ofVec2f c = baseCenter;
			auto rv1 = transformSRDPrism({pad0, pad0}, c); maybeMirror(rv1);
			auto rv2 = transformSRDPrism({W - pad0, pad0}, c); maybeMirror(rv2);
			auto rv3 = transformSRDPrism({W - pad0, H - pad0}, c); maybeMirror(rv3);
			auto rv4 = transformSRDPrism({pad0, H - pad0}, c); maybeMirror(rv4);
			ofPolyline rect;
			rect.setClosed(true);
			rect.addVertex(rv1.x, rv1.y);
			rect.addVertex(rv2.x, rv2.y);
			rect.addVertex(rv3.x, rv3.y);
			rect.addVertex(rv4.x, rv4.y);
			if (disableDotting) {
				if (spatialRainbow) {
					const float resampleSpacing = 6.0f; // pixels
					ofPolyline rectResampled = rect.getResampledBySpacing(resampleSpacing);
					const auto &pts = rectResampled.getVertices();
					for (size_t i = 0; i + 1 < pts.size(); ++i) {
						const ofVec2f &a = pts[i];
						const ofVec2f &b = pts[i+1];
						ofColor cc = colorForX((a.x + b.x) * 0.5f);
						laser.drawLine(a.x, a.y, b.x, b.y, cc, OFXLASER_PROFILE_FAST);
					}
				} else {
					laser.drawPoly(rect, colPrism, OFXLASER_PROFILE_FAST);
				}
			} else {
				// Resample to create enough points for dotting
				const float resampleSpacing = 6.0f; // pixels
				ofPolyline rectResampled = rect.getResampledBySpacing(resampleSpacing);
				drawDottedAlongPolyline(rectResampled, 0.0f);
			}
			// (debug square orientation marker removed)
			break;
		}
	case AppState::Shape::StaticWave: {
			// Sine wave across full width, centered vertically, animated by waveSpeed
			const int samples = 300;
			const float padX = 10.0f;
			const float padY = 10.0f;
			const float usableW = W - padX * 2.0f;
			const float halfH = (H - padY * 2.0f) * 0.5f;
			const float A = halfH * state->waveAmplitude.load();
			const float freq = state->waveFrequency.load();
			// Use accumulated phase for smooth speed changes without resets
			const float phase = wavePhaseRad;
			ofPolyline wave;
			wave.setClosed(false);
			for (int i = 0; i <= samples; i++) {
				float t = (float)i / (float)samples;
				float x0 = padX + t * usableW;
				float y0 = H * 0.5f + A * sinf(t * TWO_PI * freq + phase);
				ofVec2f p = transformSRDPrism({x0, y0}, baseCenter); maybeMirror(p);
				wave.addVertex(p.x, p.y);
			}
			if (disableDotting) {
				if (spatialRainbow) {
					ofPolyline res = wave.getResampledBySpacing(6.0f);
					const auto &pts = res.getVertices();
					for (size_t i = 0; i + 1 < pts.size(); ++i) {
						const ofVec2f &a = pts[i];
						const ofVec2f &b = pts[i+1];
						ofColor cc = colorForX((a.x + b.x) * 0.5f);
						laser.drawLine(a.x, a.y, b.x, b.y, cc, OFXLASER_PROFILE_FAST);
					}
				} else {
					laser.drawPoly(wave, colPrism, OFXLASER_PROFILE_FAST);
				}
			} else {
				drawDottedAlongPolyline(wave, 0.0f);
			}
			// (debug wave orientation marker removed)
			break;
		}
	}
	} // end prism loop


	// Always call send so preview + internal state continue; if blackout, temporarily zero global brightness.
	if(state->blackout.load()){
		// Hack: temporarily set manager global brightness to 0
		float prev = laser.globalBrightness.get();
		laser.globalBrightness.set(0.0f);
		laser.send();
		laser.globalBrightness.set(prev);
	} else {
		laser.send();
	}

	// draw the laser UI elements
	laser.drawUI();
}

void ofApp::exit(){
	// MIDI cleanup first (may send final OSC/messages)
	if(midiMapper){
		try { midiMapper->exit(); } catch(...) {}
		midiMapper.reset();
	}
	// ImGui / laser UI shutdown last
	try { ofxLaser::UI::shutdown(); } catch(...) {}
}

//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs& e){
    
	if(e.key==OF_KEY_TAB) {
		laser.selectNextLaser();
	}// if(e.key == ' ') {
	 //   testscale = !testscale;
	//}
    
}

void ofApp::updateOsc(){
	while(osc.hasWaitingMessages()){
		ofxOscMessage m; osc.getNextMessage(m);
		const std::string addr = m.getAddress();
		// Momentary cue handling: /cue/momentary/{n} press applies cue n temporarily until release
		if(addr.rfind("/cue/momentary/", 0) == 0){
			int idx = 0;
			try { idx = std::stoi(addr.substr(15)); } catch(...) { idx = 0; }
			float v = (m.getNumArgs()>0)? m.getArgAsFloat(0) : 1.0f; // treat no-arg as press
			bool press = v > 0.0f;
			// If we are in momentary learn mode, capture this index selection on press and return
			if(learnMomentaryArmed.load() && press){
				if(idx >=1 && idx <= (int)cues.size()){
					learnMomentaryCueIndex.store(idx);
					ofLogNotice() << "Learn(momentary): cue " << idx << " selected, waiting for MIDI note...";
				}
				continue;
			}
			if(press){
				if(!momentaryCueActive && idx >=1 && idx <= (int)cues.size()){
					// Snapshot current live state into momentaryPrevState
					momentaryPrevState.shape = state->currentShape.load();
					momentaryPrevState.colorSel = state->currentColor.load();
					momentaryPrevState.movement = state->movement.load();
					momentaryPrevState.beamFx = state->beamFx.load();
					momentaryPrevState.useCustom = state->useCustomColor.load();
					momentaryPrevState.r = state->customR.load();
					momentaryPrevState.g = state->customG.load();
					momentaryPrevState.b = state->customB.load();
					momentaryPrevState.rainbowSpeed = state->rainbowSpeed.load();
					momentaryPrevState.rainbowAmount = state->rainbowAmount.load();
					momentaryPrevState.rainbowBlend = state->rainbowBlend.load();
					momentaryPrevState.waveFrequency = state->waveFrequency.load();
					momentaryPrevState.waveAmplitude = state->waveAmplitude.load();
					momentaryPrevState.waveSpeed = state->waveSpeed.load();
					momentaryPrevState.moveSpeed = state->moveSpeed.load();
					momentaryPrevState.moveSize = state->moveSize.load();
					momentaryPrevState.rotationSpeed = state->rotationSpeed.load();
					momentaryPrevState.shapeScale = state->shapeScale.load();
					momentaryPrevState.posX = state->posNormX.load();
					momentaryPrevState.posY = state->posNormY.load();
					momentaryPrevState.dotAmount = state->dotAmount.load();
					momentaryPrevState.scanRateHz = state->scanRateHz.load();
					momentaryPrevState.populated = true; // mark snapshot valid
					prevRotationAngleRad = rotationAngleRad;
					prevWavePhaseRad = wavePhaseRad;
					prevMovePhaseRad = movePhaseRad;
					prevMoveTimeCycles = moveTimeCycles;
					bool ok = applyCue(idx);
					if(ok){
						momentaryCueActive = true;
						activeMomentaryCueIndex = idx;
						ofLogNotice() << "Momentary cue " << idx << " applied";
					}
				}
			} else {
				if(momentaryCueActive && idx == activeMomentaryCueIndex){
					// Restore snapshot
					if(momentaryPrevState.populated){
						// Directly restore selections and values (similar to applyCue)
						state->currentShape.store(momentaryPrevState.shape);
						state->currentColor.store(momentaryPrevState.colorSel);
						state->movement.store(momentaryPrevState.movement);
						state->beamFx.store(momentaryPrevState.beamFx);
						state->useCustomColor.store(momentaryPrevState.useCustom);
						state->customR.store(ofClamp(momentaryPrevState.r,0.0f,1.0f));
						state->customG.store(ofClamp(momentaryPrevState.g,0.0f,1.0f));
						state->customB.store(ofClamp(momentaryPrevState.b,0.0f,1.0f));
						state->rainbowSpeed.store(momentaryPrevState.rainbowSpeed);
						state->rainbowAmount.store(ofClamp(momentaryPrevState.rainbowAmount,0.0f,1.0f));
						state->rainbowBlend.store(ofClamp(momentaryPrevState.rainbowBlend,0.0f,1.0f));
						state->waveFrequency.store(std::max(0.1f,momentaryPrevState.waveFrequency));
						state->waveAmplitude.store(ofClamp(momentaryPrevState.waveAmplitude,0.0f,1.0f));
						state->waveSpeed.store(momentaryPrevState.waveSpeed);
						state->moveSpeed.store(momentaryPrevState.moveSpeed);
						state->moveSize.store(ofClamp(momentaryPrevState.moveSize,0.0f,1.0f));
						state->rotationSpeed.store(momentaryPrevState.rotationSpeed);
						state->rotationSpeedTarget.store(momentaryPrevState.rotationSpeed);
						state->shapeScale.store(ofClamp(momentaryPrevState.shapeScale,-1.0f,1.0f));
						state->shapeScaleTarget.store(state->shapeScale.load());
						state->posNormX.store(ofClamp(momentaryPrevState.posX,-1.0f,1.0f));
						state->posNormY.store(ofClamp(momentaryPrevState.posY,-1.0f,1.0f));
						state->posTargetX.store(state->posNormX.load());
						state->posTargetY.store(state->posNormY.load());
						state->dotAmount.store(ofClamp(momentaryPrevState.dotAmount,0.0f,1.0f));
						state->dotAmountTarget.store(state->dotAmount.load());
						// Scanrate not restored for momentary cues; remains under live control.
						// Restore accumulated phases so visual continuity returns
						rotationAngleRad = prevRotationAngleRad;
						wavePhaseRad = prevWavePhaseRad;
						movePhaseRad = prevMovePhaseRad;
						moveTimeCycles = prevMoveTimeCycles;
					}
					momentaryCueActive = false;
					activeMomentaryCueIndex = 0;
					momentaryPrevState.populated = false;
					ofLogNotice() << "Momentary cue " << idx << " released and state restored";
				}
			}
			continue;
		}
		// Learn control
		if(addr == "/learn/start"){
			// Only arm on positive/press event (value>0 or no args). Avoid duplicate logs from OFF (0) events.
			bool trigger = true;
			if(m.getNumArgs() > 0 && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
				trigger = m.getArgAsFloat(0) > 0.0f;
			} else if(m.getNumArgs() > 0 && m.getArgType(0) == OFXOSC_TYPE_INT32){
				trigger = m.getArgAsInt32(0) > 0;
			}
			if(trigger){
				if(!learnArmed.load()){
					// Cancel momentary learn if active
					learnMomentaryArmed.store(false);
					learnMomentaryCueIndex.store(0);
					learnArmed.store(true);
					learnCueIndex.store(0);
					ofLogNotice() << "Learn: armed (waiting for cue selection)";
				}
			}
			continue;
		} else if(addr == "/learn/cancel"){
			learnArmed.store(false);
			learnCueIndex.store(0);
			learnMomentaryArmed.store(false);
			learnMomentaryCueIndex.store(0);
			ofLogNotice() << "Learn: cancelled";
			continue;
		} else if(addr == "/learn/momentary/start"){
			bool trigger = true;
			if(m.getNumArgs() > 0){
				if(m.getArgType(0) == OFXOSC_TYPE_FLOAT) trigger = m.getArgAsFloat(0) > 0.0f;
				else if(m.getArgType(0) == OFXOSC_TYPE_INT32) trigger = m.getArgAsInt32(0) > 0;
			}
			if(trigger){
				learnMomentaryArmed.store(true);
				learnMomentaryCueIndex.store(0);
				// Cancel standard learn if active
				learnArmed.store(false);
				learnCueIndex.store(0);
				ofLogNotice() << "Learn(momentary): armed (select /cue/momentary/{n} then press MIDI)";
			}
			continue;
		} else if(addr == "/learn/momentary/cancel"){
			learnMomentaryArmed.store(false);
			learnMomentaryCueIndex.store(0);
			ofLogNotice() << "Learn(momentary): cancelled";
			continue;
		}
		// Cue control (save / recall / learn association)
		if(addr == "/cue/save"){
			// Arm saving: next /cue/{n} will snapshot instead of recall
			saveArmed = true;
			continue;
		} else if(addr.rfind("/cue/", 0) == 0){
			// Parse index after /cue/
			int idx = 0;
			try {
				idx = std::stoi(addr.substr(5));
			} catch(...) { idx = 0; }
			if(idx >= 1 && idx <= (int)cues.size()){
				if(learnArmed.load()){
					// Only respond to press (value>0) to avoid duplicate from release
					bool press = true;
					if(m.getNumArgs() > 0 && m.getArgType(0) == OFXOSC_TYPE_FLOAT){
						press = m.getArgAsFloat(0) > 0.0f;
					} else if(m.getNumArgs() > 0 && m.getArgType(0) == OFXOSC_TYPE_INT32){
						press = m.getArgAsInt32(0) > 0;
					}
					if(press){
						if(learnCueIndex.load() != idx){
							learnCueIndex.store(idx);
							ofLogNotice() << "Learn: cue " << idx << " selected (will map as momentary), waiting for MIDI note...";
						}
					}
					// Do NOT snapshot or apply cue while learning
					continue;
				}
				if(saveArmed){
					snapshotToCue(idx);
					saveArmed = false;
					// Persist to disk on save
					saveCuesToDisk();
				} else {
					applyCue(idx);
				}
			}
			continue;
		} else if(addr == "/ui/saveArmed"){
			// Optional UI can reflect the armed state if it sends feedback; ignore value here.
			// We only set saveArmed via /cue/save and clear on using a cue.
			continue;
		}
        
	if(addr == "/laser/shape" && m.getNumArgs() > 0){
			std::string s = ofToLower(m.getArgAsString(0));
			if(s == "line") state->currentShape.store(AppState::Shape::Line);
			else if(s == "circle") state->currentShape.store(AppState::Shape::Circle);
			else if(s == "triangle") state->currentShape.store(AppState::Shape::Triangle);
			else if(s == "square") state->currentShape.store(AppState::Shape::Square);
			else if(s == "wave" || s == "staticwave") state->currentShape.store(AppState::Shape::StaticWave);
	}
		// Dedicated endpoints for direct (argument-less) shape triggering via MIDI notes
		else if(addr == "/laser/shape/circle"){
			state->currentShape.store(AppState::Shape::Circle);
			continue;
		}
		else if(addr == "/laser/shape/line"){
			state->currentShape.store(AppState::Shape::Line);
			continue;
		}
		else if(addr == "/laser/shape/square"){
			state->currentShape.store(AppState::Shape::Square);
			continue;
		}
		else if(addr == "/laser/shape/triangle"){
			state->currentShape.store(AppState::Shape::Triangle);
			continue;
		}
		else if(addr == "/laser/shape/wave" || addr == "/laser/shape/staticwave"){
			state->currentShape.store(AppState::Shape::StaticWave);
			continue;
		}
		else if(addr == "/laser/color" && m.getNumArgs() > 0){
			// Two modes supported on the same address:
			// 1) Named color: /laser/color "blue|red|green" (disables custom)
			// 2) RGB numeric: /laser/color r g b (floats [0..1] or bytes [0..255]) -> enables custom
			auto isNumeric = [&](int i){
				auto t = m.getArgType(i);
				return t == OFXOSC_TYPE_INT32 || t == OFXOSC_TYPE_FLOAT || t == OFXOSC_TYPE_DOUBLE;
			};
			if(m.getNumArgs() >= 3 && isNumeric(0) && isNumeric(1) && isNumeric(2)){
				float r = m.getArgAsFloat(0);
				float g = m.getArgAsFloat(1);
				float b = m.getArgAsFloat(2);
				bool bytes = (r > 1.0f || g > 1.0f || b > 1.0f);
				if(bytes){ r /= 255.0f; g /= 255.0f; b /= 255.0f; }
				state->customR.store(ofClamp(r, 0.0f, 1.0f));
				state->customG.store(ofClamp(g, 0.0f, 1.0f));
				state->customB.store(ofClamp(b, 0.0f, 1.0f));
				state->useCustomColor.store(true);
				// Any static (custom) color disables rainbow
				state->rainbowAmount.store(0.0f);
				state->rainbowSpeed.store(0.0f);
			}else{
				std::string s = ofToLower(m.getArgAsString(0));
				if(s == "blue") { state->currentColor.store(AppState::ColorSel::Blue); state->useCustomColor.store(false);} 
				else if(s == "red") { state->currentColor.store(AppState::ColorSel::Red); state->useCustomColor.store(false);} 
				else if(s == "green") { state->currentColor.store(AppState::ColorSel::Green); state->useCustomColor.store(false);} 
				// Disable rainbow when a fixed color is chosen
				state->rainbowAmount.store(0.0f);
				state->rainbowSpeed.store(0.0f);
			}
		}
	// Per-channel custom RGB (knobs/sliders) – keep original behavior
		else if(addr == "/laser/color/r" && m.getNumArgs() > 0){
			float r = m.getArgAsFloat(0);
			if(r > 1.0f) r /= 255.0f;
			state->customR.store(ofClamp(r, 0.0f, 1.0f));
			state->useCustomColor.store(true);
			state->rainbowAmount.store(0.0f);
			state->rainbowSpeed.store(0.0f);
		}
		else if(addr == "/laser/color/g" && m.getNumArgs() > 0){
			float g = m.getArgAsFloat(0);
			if(g > 1.0f) g /= 255.0f;
			state->customG.store(ofClamp(g, 0.0f, 1.0f));
			state->useCustomColor.store(true);
			state->rainbowAmount.store(0.0f);
			state->rainbowSpeed.store(0.0f);
		}
		else if(addr == "/laser/color/b" && m.getNumArgs() > 0){
			float b = m.getArgAsFloat(0);
			if(b > 1.0f) b /= 255.0f;
			state->customB.store(ofClamp(b, 0.0f, 1.0f));
			state->useCustomColor.store(true);
			state->rainbowAmount.store(0.0f);
			state->rainbowSpeed.store(0.0f);
		}
	// Button palette selection (argument-less) endpoints – do NOT affect knob custom channels
		else if(addr == "/laser/color/select/red"){
			state->currentColor.store(AppState::ColorSel::Red);
			state->useCustomColor.store(false);
			state->rainbowAmount.store(0.0f);
			state->rainbowSpeed.store(0.0f);
		}
		else if(addr == "/laser/color/select/green"){
			state->currentColor.store(AppState::ColorSel::Green);
			state->useCustomColor.store(false);
			state->rainbowAmount.store(0.0f);
			state->rainbowSpeed.store(0.0f);
		}
		else if(addr == "/laser/color/select/blue"){
			state->currentColor.store(AppState::ColorSel::Blue);
			state->useCustomColor.store(false);
			state->rainbowAmount.store(0.0f);
			state->rainbowSpeed.store(0.0f);
		}
		else if(addr == "/laser/color/white"){
			// Supports both permanent (no args) and momentary (arg 1 -> white, arg 0 -> restore previous)
			static bool momentaryWhiteActive = false;
			struct PrevColorState { bool valid=false; bool prevUseCustom=false; AppState::ColorSel prevPalette=AppState::ColorSel::Red; float r=1, g=1, b=1; float rainbowAmt=0, rainbowSpd=0; };
			static PrevColorState prev;
			if(m.getNumArgs() > 0){
				float v = 0.0f;
				if(m.getArgType(0) == OFXOSC_TYPE_FLOAT) v = m.getArgAsFloat(0);
				else if(m.getArgType(0) == OFXOSC_TYPE_INT32) v = (float)m.getArgAsInt32(0);
				if(v > 0.0f){
					// Engage momentary white if not already
					if(!momentaryWhiteActive){
						prev.valid = true;
						prev.prevUseCustom = state->useCustomColor.load();
						prev.prevPalette = (AppState::ColorSel)state->currentColor.load();
						prev.r = state->customR.load();
						prev.g = state->customG.load();
						prev.b = state->customB.load();
						prev.rainbowAmt = state->rainbowAmount.load();
						prev.rainbowSpd = state->rainbowSpeed.load();
						// Apply white
						state->customR.store(1.0f);
						state->customG.store(1.0f);
						state->customB.store(1.0f);
						state->useCustomColor.store(true);
						state->rainbowAmount.store(0.0f);
						state->rainbowSpeed.store(0.0f);
						momentaryWhiteActive = true;
						ofLogNotice() << "Momentary white engaged";
					}
				} else {
					// Release
					if(momentaryWhiteActive){
						if(prev.valid){
							state->useCustomColor.store(prev.prevUseCustom);
							state->currentColor.store(prev.prevPalette);
							state->customR.store(prev.r);
							state->customG.store(prev.g);
							state->customB.store(prev.b);
							state->rainbowAmount.store(prev.rainbowAmt);
							state->rainbowSpeed.store(prev.rainbowSpd);
						}
						momentaryWhiteActive = false;
						ofLogNotice() << "Momentary white released";
					}
				}
			} else {
				// Permanent set to white (legacy behavior)
				state->customR.store(1.0f);
				state->customG.store(1.0f);
				state->customB.store(1.0f);
				state->useCustomColor.store(true);
				state->rainbowAmount.store(0.0f);
				state->rainbowSpeed.store(0.0f);
				momentaryWhiteActive = false; // treat as base, no revert target
				prev.valid = false;
			}
		}
		else if(addr == "/laser/color/select/white"){
			// White as palette: disable custom so knobs can re-activate when used again
			state->customR.store(1.0f);
			state->customG.store(1.0f);
			state->customB.store(1.0f);
			state->useCustomColor.store(true); // keep as custom so it actually outputs white
			state->rainbowAmount.store(0.0f);
			state->rainbowSpeed.store(0.0f);
		}
		else if(addr == "/laser/wave/frequency" && m.getNumArgs() > 0){
			state->waveFrequency.store(std::max(0.1f, m.getArgAsFloat(0)));
		}
		else if(addr == "/laser/wave/amplitude" && m.getNumArgs() > 0){
			state->waveAmplitude.store(ofClamp(m.getArgAsFloat(0), 0.0f, 1.0f));
		}
		else if(addr == "/laser/wave/speed" && m.getNumArgs() > 0){
			// Update only the speed; phase continuity is maintained by accumulator in update()
			state->waveSpeed.store(m.getArgAsFloat(0));
		}
		// Movement UI
		else if(addr == "/move/mode" && m.getNumArgs() > 0){
			std::string s = ofToLower(m.getArgAsString(0));
			if(s=="none" || s=="off") {
				state->movement.store(AppState::Movement::None);
			} else {
				if(s=="circle") state->movement.store(AppState::Movement::Circle);
				else if(s=="pan") state->movement.store(AppState::Movement::Pan);
				else if(s=="tilt") state->movement.store(AppState::Movement::Tilt);
				else if(s=="eight" || s=="figure8" || s=="8") state->movement.store(AppState::Movement::Eight);
				else if(s=="random") state->movement.store(AppState::Movement::Random);

				// Ensure movement starts even if user hasn't adjusted knobs yet
				// Only apply defaults when current values are effectively zero
				constexpr float kEps = 1e-4f;
				constexpr float kDefaultMoveSize = 0.2f;   // gentle amplitude
				constexpr float kDefaultMoveSpeed = 0.12f; // slow cycles per second
				if(state->moveSize.load() <= kEps) {
					state->moveSize.store(kDefaultMoveSize);
				}
				if(fabsf(state->moveSpeed.load()) <= kEps) {
					state->moveSpeed.store(kDefaultMoveSpeed);
				}
			}
		}
		else if(addr == "/move/select/circle"){
			state->movement.store(AppState::Movement::Circle);
		}
		else if(addr == "/move/select/pan"){
			state->movement.store(AppState::Movement::Pan);
		}
		else if(addr == "/move/select/tilt"){
			state->movement.store(AppState::Movement::Tilt);
		}
		else if(addr == "/move/select/eight"){
			state->movement.store(AppState::Movement::Eight);
		}
		else if(addr == "/move/select/random"){
			state->movement.store(AppState::Movement::Random);
		}
		else if(addr == "/move/size" && m.getNumArgs() > 0){
			float v = m.getArgAsFloat(0);
			if(v > 1.0f) v /= 255.0f; // allow 0..255
			state->moveSize.store(ofClamp(v, 0.0f, 1.0f));
		}
		else if(addr == "/laser/axis/invert/x" && m.getNumArgs() > 0){
			bool inv = m.getArgAsFloat(0) > 0.5f;
			state->invertX.store(inv);
		}
		else if(addr == "/laser/axis/invert/x/hold" && m.getNumArgs() > 0){
			bool on = m.getArgAsFloat(0) > 0.5f;
			state->holdInvertX.store(on);
		}
		else if(addr == "/laser/color/flash/white/hold" && m.getNumArgs() > 0){
			bool on = m.getArgAsFloat(0) > 0.5f;
			state->holdWhiteFlash.store(on);
		}
		else if(addr == "/laser/blackout/hold" && m.getNumArgs() > 0){
			bool on = m.getArgAsFloat(0) > 0.5f;
			state->blackout.store(on);
		}
		else if(addr == "/motion/hold" && m.getNumArgs() > 0){
			bool on = m.getArgAsFloat(0) > 0.5f;
			bool prev = state->motionHold.load();
			if(on && !prev){
				// capture current speeds
				state->heldRotationSpeed.store(state->rotationSpeed.load());
				state->heldMoveSpeed.store(state->moveSpeed.load());
				state->heldWaveSpeed.store(state->waveSpeed.load());
			} else if(!on && prev){
				// restore cached speeds ONLY if user hasn't changed them while paused
				// (Simpler: always restore)
				state->rotationSpeed.store(state->heldRotationSpeed.load());
				state->moveSpeed.store(state->heldMoveSpeed.load());
				state->waveSpeed.store(state->heldWaveSpeed.load());
			}
			state->motionHold.store(on);
			ofLogNotice() << "Motion hold " << (on?"ENGAGED":"RELEASED");
		}
		else if(addr == "/move/speed" && m.getNumArgs() > 0){
			// cycles per second; allow negative for reverse direction
			state->moveSpeed.store(m.getArgAsFloat(0));
		}
		else if((addr == "/laser/brightness" || addr == "/laser/master/brightness") && m.getNumArgs() > 0){
			// Accept either [0..1] float or [0..255] int/float. Clamp to [0..1].
			float v = m.getArgAsFloat(0);
			if(v > 1.0f) v /= 255.0f;
			state->masterBrightness.store(ofClamp(v, 0.0f, 1.0f));
		}
	// (legacy simple /laser/rotation/speed handler removed – unified advanced mapping lives further below)
		else if(addr == "/laser/shape/scale" && m.getNumArgs() > 0){
			// Expect normalized range [-1..1]; we smooth toward it.
			float v = ofClamp(m.getArgAsFloat(0), -1.0f, 1.0f);
			state->shapeScaleTarget.store(v);
			hasScaleInput = true; // mark that scale has been explicitly set this session
		}
	// MIDI removed: /midi/cc ignored
	// MIDI and flash learn removed
		else if(addr == "/flash/release_ms" && m.getNumArgs() > 0){
			// Set release time in milliseconds; 0 = instant (legacy behavior)
			int ms = (int)roundf(m.getArgAsFloat(0));
			ms = ofClamp(ms, 0, 60000); // clamp to 60s max
			flashReleaseMs.store(ms);
			ofLogNotice() << "Flash release time set to " << ms << " ms";
		}
		else if(addr == "/flash" && m.getNumArgs() > 0){
			// Optional direct control from UI button: 1=press, 0=release
			float v = m.getArgAsFloat(0);
			bool press = (v != 0.0f);
			if(press){
				flashPrevBrightness = state->masterBrightness.load();
				flashActive = true;
				flashDecaying = false;
			} else {
				flashActive = false;
				int ms = std::max(0, flashReleaseMs.load());
				if(ms > 0){
					flashDecayFrom = 1.0f;
					flashDecayStartMs = ofGetElapsedTimeMillis();
					flashDecaying = true;
				} else {
					state->masterBrightness.store(ofClamp(flashPrevBrightness, 0.0f, 1.0f));
					flashDecaying = false;
				}
			}
		}
		else if(addr == "/laser/position" && m.getNumArgs() >= 2){
			// Two floats in [-1..+1] for x and y. -1 = -500%, +1 = +500% of half-dimension.
			float x = ofClamp(m.getArgAsFloat(0), -1.0f, 1.0f);
			float y = ofClamp(m.getArgAsFloat(1), -1.0f, 1.0f);
			// Write to targets; if first movement after inactivity, also seed current to avoid lag.
			state->posTargetX.store(x);
			state->posTargetY.store(y);
		}
		else if(addr == "/laser/position/x" && m.getNumArgs() > 0){
			float x = ofClamp(m.getArgAsFloat(0), -1.0f, 1.0f);
			state->posTargetX.store(x);
		}
		else if(addr == "/laser/position/y" && m.getNumArgs() > 0){
			float y = ofClamp(m.getArgAsFloat(0), -1.0f, 1.0f);
			state->posTargetY.store(y);
		}
		else if(addr == "/laser/dotted" && m.getNumArgs() > 0){
			float v = m.getArgAsFloat(0);
			if(v > 1.0f) v /= 255.0f; // allow 0..255 input
			state->dotAmountTarget.store(ofClamp(v, 0.0f, 1.0f));
		}
	else if(addr == "/laser/scanrate" && m.getNumArgs() > 0){
			// Global DAC point rate override (safe range 3000..20000)
			// Updated semantics (2025-08-21):
			//  v == -1              -> disable override (revert to per-laser defaults)
			//  0..1                 -> normalized (0 -> minPPS=2000, 1 -> maxPPS=20000)
			//  1 < v < minPPS       -> treated as controller domain value (e.g. 0..127 / 0..255); optional 2nd arg = domain max
			//  v >= minPPS          -> absolute PPS request (clamped)
			//  Startup default (if none applied): 20000 (handled in setup())
			const int minPPS = 2000; // lowered from 3000
			const int maxPPS = 20000;
			float v = m.getArgAsFloat(0);
			int target = 0; // 0 => disabled (only when v < 0)
			if(v < 0.0f && v > -1.5f){
				target = 0; // disable
			} else if(v <= 1.0f){
				// normalized including 0 mapping to minPPS
				float n = ofClamp(v, 0.0f, 1.0f);
				target = (int)std::round(ofLerp((float)minPPS, (float)maxPPS, n));
			} else if(v < (float)minPPS){
				// controller domain
				float controlMax = (m.getNumArgs() > 1)? m.getArgAsFloat(1) : 127.0f;
				if(controlMax < 1.0f) controlMax = 1.0f;
				float n = ofClamp(v, 0.0f, controlMax) / controlMax; // 0..1
				target = (int)std::round(ofLerp((float)minPPS,(float)maxPPS,n));
			} else {
				// absolute PPS
				target = (int)std::round(v);
			}
			if(target>0) target = ofClamp(target, minPPS, maxPPS);
			int prevTarget = ppsTarget;
			ppsTarget = target; // set desired target
			ofLogNotice() << "/laser/scanrate request v=" << v << " -> target=" << target << " (prevTarget=" << prevTarget << ") current=" << ppsCurrent;
			if(target==0){
				// disable immediately
				int num = laser.getNumLasers();
				for(int i=0;i<num;++i){
					try { laser.getLaser(i).setPpsOverride(0); } catch(...) {}
				}
				ppsCurrent = 0;
				state->scanRateHz.store(0.0f);
				ofLogNotice() << "PPS override disabled";
			}
		}

		else if(addr == "/laser/rainbow/amount" && m.getNumArgs() > 0){
			// 0..1 spatial size; no auto-setting of speed (default stays 0 until user turns knob)
			float v = ofClamp(m.getArgAsFloat(0), 0.0f, 1.0f);
			state->rainbowAmount.store(v);
		}
		else if(addr == "/laser/rainbow/preset/slowfull"){
			// Preset button: enable full spatial rainbow with very slow gentle animation.
			// Chosen values: amount=1.0 (max width), blend=1.0 (smooth), speed=0.05 cps (~20s per full cycle)
			state->rainbowAmount.store(0.95f);
			state->rainbowBlend.store(1.0f);
			state->rainbowSpeed.store(0.05f);
			ofLogNotice() << "Applied rainbow preset slowfull (amount=1 blend=1 speed=0.05cps)";
		}
		else if(addr == "/laser/rainbow/speed" && m.getNumArgs() > 0){
			// Bi-directional speed control using knob left/right.
			// Expect input in [-1..1] or 0..255 (centered at 127/128) and map to cycles/sec.
			float raw = m.getArgAsFloat(0);
			// Normalize possible byte input
			if (raw > 1.0f) {
				// Map 0..255 -> [-1..1]
				raw = ofMap(ofClamp(raw, 0.0f, 255.0f), 0.0f, 255.0f, -1.0f, 1.0f);
			}
			raw = ofClamp(raw, -1.0f, 1.0f);
			// Map to cycles/sec: 0 at center, up to ~2 cps at extremes (tweakable)
			float cps = raw * 2.0f;
			state->rainbowSpeed.store(cps);
		}
		else if(addr == "/laser/rainbow/blend" && m.getNumArgs() > 0){
			// 0..1: 0 = sharp color bands, 1 = smooth gradient
			float v = ofClamp(m.getArgAsFloat(0), 0.0f, 1.0f);
			state->rainbowBlend.store(v);
		}
	else if(addr == "/laser/rotation/speed" && m.getNumArgs() > 0){
			// Enhanced mapping (2025-08-24): much slower minimum speeds with more resolution.
			// Raw expected in [-1..1] (centered knob). If we get 0..255 we normalize.
			float raw = m.getArgAsFloat(0);
			if(raw > 1.5f){ raw = ofMap(ofClamp(raw,0.0f,255.0f), 0,255,-1,1); }
			raw = ofClamp(raw,-1.0f,1.0f);
			float sign = (raw < 0.0f) ? -1.0f : 1.0f;
			float n = fabsf(raw);
			// Parameters:
			const float zone = 0.18f;       // enlarged fine zone (18% travel)
			const float slowMax = 0.25f;     // rps at end of fine zone (~15 deg/sec)
			const float superSlowRps = 0.02f;// lower bound we can still reach gradually
			const float maxRps = 45.0f;      // unchanged top speed
			float rps;
			if(n < 1e-6f){
				rps = 0.0f;
			} else if(n <= zone){
				// Use cubic easing to give even more precision near 0.
				float t = n / zone;               // 0..1
				float curve = t * t * t;          // cubic
				// Blend from superSlowRps to slowMax (so first movement already visible but very slow)
				rps = ofLerp(superSlowRps, slowMax, curve);
				// If user barely moves (t < ~0.05) fade back toward 0 to keep a tangible deadband
				if(t < 0.05f){
					float dd = t / 0.05f; // 0..1
					rps *= dd;            // soft entry
				}
			} else {
				// Remaining travel accelerates exponentially from slowMax to maxRps
				float t = (n - zone) / (1.0f - zone); // 0..1
				const float k = 4.5f; // slightly steeper mid acceleration
				float accel = (expf(k * t) - 1.0f) / (expf(k) - 1.0f);
				rps = slowMax + accel * (maxRps - slowMax);
			}
			rps *= sign;
			if(fabsf(rps) < 0.0002f) rps = 0.0f; // tiny deadband cleanup
			state->rotationSpeedTarget.store(rps);
			ofLogNotice() << "/laser/rotation/speed raw=" << raw << " -> rps=" << rps;
		}
		// Beam FX selection (mutually exclusive)
		else if(addr == "/beam/select/prisma"){
			// Treat non-zero argument (or lack of args) as ON; ignore explicit 0 so forced-off from exclusive group doesn't re-trigger.
			if(m.getNumArgs()==0 || m.getArgAsFloat(0) != 0.0f){
				state->beamFx.store(AppState::BeamFx::Prisma);
			}
		}
		else if(addr == "/beam/select/none"){
			if(m.getNumArgs()==0 || m.getArgAsFloat(0) != 0.0f){
				state->beamFx.store(AppState::BeamFx::None);
			}
		}
		else if(addr == "/midi/reload"){
			if(midiMapper){
				midiMapper->loadConfig();
				ofLogNotice() << "MIDI mapping reloaded via /midi/reload";
			}
		}
		else if(addr == "/midi/dump"){
			if(midiMapper){
				midiMapper->dumpNoteMappings();
			}
		}
	}
}
void ofApp::snapshotToCue(int idx){
	if(idx < 1 || idx > (int)cues.size()) return;
	CueState cs;
	cs.shape = state->currentShape.load();
	cs.colorSel = state->currentColor.load();
	cs.movement = state->movement.load();
	cs.beamFx = state->beamFx.load();
	cs.useCustom = state->useCustomColor.load();
	cs.r = state->customR.load();
	cs.g = state->customG.load();
	cs.b = state->customB.load();
	cs.rainbowSpeed = state->rainbowSpeed.load();
	cs.rainbowAmount = state->rainbowAmount.load();
	cs.rainbowBlend = state->rainbowBlend.load();
	cs.waveFrequency = state->waveFrequency.load();
	cs.waveAmplitude = state->waveAmplitude.load();
	cs.waveSpeed = state->waveSpeed.load();
	cs.moveSpeed = state->moveSpeed.load();
	cs.moveSize = state->moveSize.load();
	cs.rotationSpeed = state->rotationSpeed.load();
	cs.shapeScale = state->shapeScale.load();
	cs.posX = state->posNormX.load();
	cs.posY = state->posNormY.load();
	cs.dotAmount = state->dotAmount.load();
	cs.scanRateHz = state->scanRateHz.load();
	cs.populated = true;
	cues[idx-1] = cs;
	ofLogNotice() << "Saved cue " << idx;
}

bool ofApp::applyCue(int idx){
	if(idx < 1 || idx > (int)cues.size()) return false;
	CueState cs = cues[idx-1]; // make mutable copy for clamping adjustments
	if(!cs.populated) {
		ofLogWarning() << "Cue " << idx << " empty";
		return false;
	}
	state->currentShape.store(cs.shape);
	state->currentColor.store(cs.colorSel);
	state->movement.store(cs.movement);
	state->beamFx.store(cs.beamFx);
	state->useCustomColor.store(cs.useCustom);
	state->customR.store(ofClamp(cs.r, 0.0f, 1.0f));
	state->customG.store(ofClamp(cs.g, 0.0f, 1.0f));
	state->customB.store(ofClamp(cs.b, 0.0f, 1.0f));
	state->rainbowSpeed.store(cs.rainbowSpeed);
	state->rainbowAmount.store(ofClamp(cs.rainbowAmount, 0.0f, 1.0f));
	state->rainbowBlend.store(ofClamp(cs.rainbowBlend, 0.0f, 1.0f));
	state->waveFrequency.store(std::max(0.1f, cs.waveFrequency));
	state->waveAmplitude.store(ofClamp(cs.waveAmplitude, 0.0f, 1.0f));
	state->waveSpeed.store(cs.waveSpeed);
	state->moveSpeed.store(cs.moveSpeed);
	state->moveSize.store(ofClamp(cs.moveSize, 0.0f, 1.0f));
	{
		float rs = cs.rotationSpeed;
		// Expanded range: live mapping supports up to ±45 rps; only clamp beyond that for safety.
		const float maxRps = 45.0f;
		if(rs < -maxRps || rs > maxRps){
			float clamped = ofClamp(rs, -maxRps, maxRps);
			ofLogNotice() << "Cue rotationSpeed clamped " << rs << " -> " << clamped;
			rs = clamped;
		}
	state->rotationSpeed.store(rs);
	state->rotationSpeedTarget.store(rs);
	// Reset accumulated rotation so each cue starts from a neutral orientation.
	rotationAngleRad = 0.0f;
	}
	state->shapeScale.store(ofClamp(cs.shapeScale, -1.0f, 1.0f));
	state->shapeScaleTarget.store(state->shapeScale.load());
	hasScaleInput = true;
	state->posNormX.store(ofClamp(cs.posX, -1.0f, 1.0f));
	state->posNormY.store(ofClamp(cs.posY, -1.0f, 1.0f));
	// Also update smoothing targets so we don't ease from previous cue unexpectedly.
	state->posTargetX.store(state->posNormX.load());
	state->posTargetY.store(state->posNormY.load());
	state->dotAmount.store(ofClamp(cs.dotAmount, 0.0f, 1.0f));
	state->dotAmountTarget.store(state->dotAmount.load());
	// Ignore stored scanRateHz when loading cues; scanrate is only changed by live OSC/MIDI (/laser/scanrate fader).
	ofLogNotice() << "Loaded cue " << idx << " (scanrate unchanged)";
	return true;
}

// ---------- Persistence ----------

std::string ofApp::shapeToString(AppState::Shape s){
	switch(s){
		case AppState::Shape::Circle: return "circle";
		case AppState::Shape::Line: return "line";
		case AppState::Shape::Triangle: return "triangle";
		case AppState::Shape::Square: return "square";
		case AppState::Shape::StaticWave: return "staticwave";
	}
	return "circle";
}

AppState::Shape ofApp::shapeFromString(const std::string& s){
	std::string t = ofToLower(s);
	if(t=="line") return AppState::Shape::Line;
	if(t=="triangle") return AppState::Shape::Triangle;
	if(t=="square") return AppState::Shape::Square;
	if(t=="staticwave" || t=="wave") return AppState::Shape::StaticWave;
	return AppState::Shape::Circle;
}

std::string ofApp::colorToString(AppState::ColorSel c){
	switch(c){
		case AppState::ColorSel::Red: return "red";
		case AppState::ColorSel::Green: return "green";
		case AppState::ColorSel::Blue: default: return "blue";
	}
}

AppState::ColorSel ofApp::colorFromString(const std::string& s){
	std::string t = ofToLower(s);
	if(t=="red") return AppState::ColorSel::Red;
	if(t=="green") return AppState::ColorSel::Green;
	return AppState::ColorSel::Blue;
}

void ofApp::saveCuesToDisk(){
	ofJson root;
	root["version"] = 1;
	auto& arr = root["cues"] = ofJson::array();
	for(size_t i=0;i<cues.size();++i){
		const auto& cs = cues[i];
		ofJson j;
		j["index"] = (int)i+1;
		j["populated"] = cs.populated;
		if(cs.populated){
			j["shape"] = shapeToString(cs.shape);
			j["colorSel"] = colorToString(cs.colorSel);
			// movement
			switch(cs.movement){
				case AppState::Movement::Circle: j["movement"] = "circle"; break;
				case AppState::Movement::Pan: j["movement"] = "pan"; break;
				case AppState::Movement::Tilt: j["movement"] = "tilt"; break;
				case AppState::Movement::Eight: j["movement"] = "eight"; break;
				case AppState::Movement::Random: j["movement"] = "random"; break;
				default: j["movement"] = "none"; break;
			}
			// beam fx
			switch(cs.beamFx){
				case AppState::BeamFx::Prisma: j["beamFx"] = "prisma"; break;
				default: j["beamFx"] = "none"; break;
			}
			j["useCustom"] = cs.useCustom;
			j["r"] = cs.r; j["g"] = cs.g; j["b"] = cs.b;
			j["waveFrequency"] = cs.waveFrequency;
			j["waveAmplitude"] = cs.waveAmplitude;
			j["waveSpeed"] = cs.waveSpeed;
			j["rainbowSpeed"] = cs.rainbowSpeed;
			j["rainbowAmount"] = cs.rainbowAmount;
			j["rainbowBlend"] = cs.rainbowBlend;
			j["rotationSpeed"] = cs.rotationSpeed;
			j["moveSpeed"] = cs.moveSpeed;
			j["moveSize"] = cs.moveSize;
			j["shapeScale"] = cs.shapeScale;
			j["posX"] = cs.posX; j["posY"] = cs.posY;
			j["dotAmount"] = cs.dotAmount;
			j["scanRateHz"] = cs.scanRateHz;
		}
		arr.push_back(j);
	}
	const std::string path = ofToDataPath(cuesFileName, true);
	ofSavePrettyJson(path, root);
	ofLogNotice() << "Cues saved to " << path;
}

void ofApp::loadCuesFromDisk(){
	const std::string path = ofToDataPath(cuesFileName, true);
	if(!ofFile::doesFileExist(path)){
		ofLogNotice() << "No cues file found (" << path << ")";
		return;
	}
	ofJson root;
	try{
		root = ofLoadJson(path);
	}catch(...){
		ofLogError() << "Failed to parse cues file: " << path;
		return;
	}
	if(!root.contains("cues") || !root["cues"].is_array()){
		ofLogWarning() << "Cues file missing 'cues' array";
		return;
	}
	for(const auto& j : root["cues"]) {
		int idx = j.value("index", 0);
		if(idx < 1 || idx > (int)cues.size()) continue;
		auto& cs = cues[idx-1];
		cs.populated = j.value("populated", false);
		if(!cs.populated) continue;
		cs.shape = shapeFromString(j.value("shape", "circle"));
		cs.colorSel = colorFromString(j.value("colorSel", "blue"));
		// movement
		{
			std::string mv = ofToLower(j.value("movement", "none"));
			if(mv=="circle") cs.movement = AppState::Movement::Circle;
			else if(mv=="pan") cs.movement = AppState::Movement::Pan;
			else if(mv=="tilt") cs.movement = AppState::Movement::Tilt;
			else if(mv=="eight" || mv=="figure8" || mv=="8") cs.movement = AppState::Movement::Eight;
			else if(mv=="random") cs.movement = AppState::Movement::Random;
			else cs.movement = AppState::Movement::None;
		}
		// beam fx
		{
			std::string bfx = ofToLower(j.value("beamFx", "none"));
			if(bfx=="prisma") cs.beamFx = AppState::BeamFx::Prisma; else cs.beamFx = AppState::BeamFx::None;
		}
		cs.useCustom = j.value("useCustom", false);
		cs.r = j.value("r", 0.0f); cs.g = j.value("g", 0.0f); cs.b = j.value("b", 0.0f);
		cs.waveFrequency = j.value("waveFrequency", 1.0f);
		cs.waveAmplitude = j.value("waveAmplitude", 0.45f);
		cs.waveSpeed = j.value("waveSpeed", 0.0f);
	cs.rainbowSpeed = j.value("rainbowSpeed", 0.0f);
	cs.rainbowAmount = j.value("rainbowAmount", 0.0f);
	cs.rainbowBlend = j.value("rainbowBlend", 1.0f);
	cs.rotationSpeed = j.value("rotationSpeed", 0.0f);
	cs.moveSpeed = j.value("moveSpeed", 0.0f);
	cs.moveSize = j.value("moveSize", 0.0f);
		cs.shapeScale = j.value("shapeScale", 0.0f);
		cs.posX = j.value("posX", 0.0f); cs.posY = j.value("posY", 0.0f);
		cs.dotAmount = j.value("dotAmount", 1.0f);
	cs.scanRateHz = j.value("scanRateHz", 0.0f);
	}
	ofLogNotice() << "Cues loaded from " << path;
}

// ---------------- Joystick cue mapping implementation ----------------
void ofApp::loadJoystickCueMappings(){
	joystickCueMaps.clear();
	std::string path = ofToDataPath("joystick_cues.json", true);
	if(!ofFile::doesFileExist(path)) return;
	try {
		ofJson arr = ofLoadJson(path);
		if(arr.is_array()){
			for(const auto &e : arr){
				JoystickCueMap m; m.joystick = e.value("joystick", -1); m.button = e.value("button", -1); m.cueIndex = e.value("cueIndex", 0); m.momentary = e.value("momentary", true);
				if(m.joystick>=0 && m.button>=0 && m.cueIndex>0) joystickCueMaps.push_back(m);
			}
		}
		ofLogNotice() << "Loaded " << joystickCueMaps.size() << " joystick cue mappings";
	} catch(...) { ofLogError() << "Failed parsing joystick_cues.json"; }
}

void ofApp::saveJoystickCueMappings(){
	ofJson arr = ofJson::array();
	for(const auto &m : joystickCueMaps){
		ofJson e; e["joystick"] = m.joystick; e["button"] = m.button; e["cueIndex"] = m.cueIndex; e["momentary"] = m.momentary; arr.push_back(e);
	}
	std::string path = ofToDataPath("joystick_cues.json", true);
	ofSavePrettyJson(path, arr);
	ofLogNotice() << "Saved joystick cue mappings to " << path;
}

void ofApp::addOrUpdateMomentaryCueJoystickMapping(int joystick, int button, int cueIndex){
	if(cueIndex < 1) return;
	for(auto &m : joystickCueMaps){
		if(m.joystick==joystick && m.button==button){
			m.cueIndex = cueIndex; m.momentary = true; saveJoystickCueMappings();
			ofLogNotice() << "Updated joystick mapping j" << joystick << " btn" << button << " -> cue " << cueIndex;
			return;
		}
	}
	JoystickCueMap nm; nm.joystick=joystick; nm.button=button; nm.cueIndex=cueIndex; nm.momentary=true;
	joystickCueMaps.push_back(nm);
	saveJoystickCueMappings();
	ofLogNotice() << "Added joystick mapping j" << joystick << " btn" << button << " -> cue " << cueIndex;
}

void ofApp::pollJoysticksForLearningAndTriggers(){
#if HAVE_OFXJOYSTICK
	// Initialize once (lazy) if needed
	static bool joyInit = false;
	if(!joyInit){
		ofxJoy.setup();
		joyInit = true;
	}
	// Iterate devices
	for(size_t d=0; d<ofxJoy.getNumJoysticks(); ++d){
		auto &js = ofxJoy.getJoystick(d);
		js.update();
		int jid = (int)d; // internal index as id
		// Buttons
		for(size_t b=0; b<js.getNumButtons(); ++b){
			bool pressed = js.getButton(b); // bool state
			uint32_t key = ((uint32_t)jid<<16) | (uint32_t)b;
			bool prev = false; auto it = joyPrevButtonState.find(key); if(it!=joyPrevButtonState.end()) prev = it->second;
			if(pressed && !prev){
				if(learnArmed.load() && learnCueIndex.load()>0){
					int cueIdx = learnCueIndex.load();
					addOrUpdateMomentaryCueJoystickMapping(jid, (int)b, cueIdx);
					ofLogNotice() << "Learn: mapped joystick " << jid << " button " << b << " -> momentary cue " << cueIdx;
					learnArmed.store(false); learnCueIndex.store(0);
				} else if(learnMomentaryArmed.load() && learnMomentaryCueIndex.load()>0){
					int cueIdx = learnMomentaryCueIndex.load();
					addOrUpdateMomentaryCueJoystickMapping(jid, (int)b, cueIdx);
					ofLogNotice() << "Learn(momentary): mapped joystick " << jid << " button " << b << " -> cue " << cueIdx;
					learnMomentaryArmed.store(false); learnMomentaryCueIndex.store(0);
				}
			}
			if(pressed != prev){
				for(const auto &m : joystickCueMaps){
					if(m.joystick==jid && m.button==(int)b && m.momentary){
						std::string addr = "/cue/momentary/" + ofToString(m.cueIndex);
						ofxOscMessage om; om.setAddress(addr); om.addFloatArg(pressed?1.f:0.f); joystickOscSender.sendMessage(om,false);
					}
				}
			}
			joyPrevButtonState[key] = pressed;
		}
	}
#else
	// GLFW fallback (as before)
	#if defined(TARGET_OSX) || defined(TARGET_LINUX) || defined(TARGET_WIN32)
	for(int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid){
		if(!glfwJoystickPresent(jid)) continue;
		int buttonCount = 0; const unsigned char *buttons = glfwGetJoystickButtons(jid, &buttonCount);
		if(!buttons || buttonCount<=0) continue;
		for(int b=0; b<buttonCount; ++b){
			uint32_t key = ((uint32_t)jid<<16) | (uint32_t)b;
			bool pressed = buttons[b] == GLFW_PRESS;
			bool prev = false; auto it = joyPrevButtonState.find(key); if(it!=joyPrevButtonState.end()) prev = it->second;
			if(pressed && !prev){
				if(learnArmed.load() && learnCueIndex.load()>0){
					int cueIdx = learnCueIndex.load();
					addOrUpdateMomentaryCueJoystickMapping(jid, b, cueIdx);
					ofLogNotice() << "Learn: mapped joystick " << jid << " button " << b << " -> momentary cue " << cueIdx;
					learnArmed.store(false); learnCueIndex.store(0);
				} else if(learnMomentaryArmed.load() && learnMomentaryCueIndex.load()>0){
					int cueIdx = learnMomentaryCueIndex.load();
					addOrUpdateMomentaryCueJoystickMapping(jid, b, cueIdx);
					ofLogNotice() << "Learn(momentary): mapped joystick " << jid << " button " << b << " -> cue " << cueIdx;
					learnMomentaryArmed.store(false); learnMomentaryCueIndex.store(0);
				}
			}
			if(pressed != prev){
				for(const auto &m : joystickCueMaps){
					if(m.joystick==jid && m.button==b && m.momentary){
						std::string addr = "/cue/momentary/" + ofToString(m.cueIndex);
						ofxOscMessage om; om.setAddress(addr); om.addFloatArg(pressed?1.f:0.f); joystickOscSender.sendMessage(om,false);
					}
				}
			}
			joyPrevButtonState[key] = pressed;
		}
	}
	#endif
#endif
}


