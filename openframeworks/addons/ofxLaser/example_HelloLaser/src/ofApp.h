
#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"
#include "ofxOsc.h"
#include "AppState.h"
#include <array>
// MIDI -> OSC mapper
#include "MidiToOscMapper.h"
// MIDI removed

// Optional: use ofxJoystick addon if available (user can add addon later).
#ifdef __has_include
#if __has_include("ofxJoystick.h")
#define HAVE_OFXJOYSTICK 1
#include "ofxJoystick.h"
#else
#define HAVE_OFXJOYSTICK 0
#endif
#else
#define HAVE_OFXJOYSTICK 0
#endif


class ofApp : public ofBaseApp{
	
public:
    
	void setup();
	void update();
	void draw();
	void exit();
	
	// removed: star poly helper
    
	void keyPressed(ofKeyEventArgs& e);
    
	ofxLaser::Manager laser;
	std::shared_ptr<AppState> state = std::make_shared<AppState>();

	// OSC
	ofxOscReceiver osc;
	int oscPort = 9000; // default
	void updateOsc();

	// MIDI removed
	// MIDI -> OSC mapper (APC40 mkII master fader -> /laser/brightness)
	std::unique_ptr<MidiToOscMapper> midiMapper;

	// MIDI cue learn state
	std::atomic<bool> learnArmed{false};          // true after /learn/start until mapping completes or cancel
	std::atomic<int> learnCueIndex{0};            // selected cue (1-based) while waiting for MIDI note
	// Momentary cue learn state (/learn/momentary/start)
	std::atomic<bool> learnMomentaryArmed{false};
	std::atomic<int> learnMomentaryCueIndex{0};    // selected momentary cue index while waiting for MIDI note

	// Flash direct control via OSC
	bool flashActive = false;
	float flashPrevBrightness = 0.0f;
	// Flash release behavior (fade to 0 over this many milliseconds on release)
	std::atomic<int> flashReleaseMs{150}; // default 150 ms, 0 = instant
	bool flashDecaying = false;
	uint64_t flashDecayStartMs = 0;
	float flashDecayFrom = 1.0f;
    
	// Cues: snapshot and recall full AppState via OSC (/cue/save then /cue/{n})
	struct CueState {
		// AppState snapshot values (non-atomic copies)
		AppState::Shape shape;
		AppState::ColorSel colorSel;
	AppState::Movement movement;
	AppState::BeamFx beamFx; // new beam effect state
		bool useCustom;
		float r, g, b; // custom color [0..1]
		float rainbowSpeed;
		float rainbowAmount;
		float rainbowBlend;
		float waveFrequency;
		float waveAmplitude;
		float waveSpeed;
	float moveSpeed;
	float moveSize;
		float rotationSpeed;
		float shapeScale; // [-1..1]
		float posX, posY; // [-1..1]
		float dotAmount; // [0..1]
		float scanRateHz; // stored as actual PPS value (legacy key name)
		bool populated = false;
	};
    
	// Small bank of cues (1-based indexing for UI). Expanded to 30 to match Open Stage Control UI (3x10)
	std::array<CueState,30> cues;
	bool saveArmed = false;
	void snapshotToCue(int idx);
	bool applyCue(int idx);
    
	// Persistence
	const std::string cuesFileName = "cues.json"; // saved under data/ by ofToDataPath
	void saveCuesToDisk();
	void loadCuesFromDisk();
    
	static std::string shapeToString(AppState::Shape s);
	static AppState::Shape shapeFromString(const std::string&);
	static std::string colorToString(AppState::ColorSel c);
	static AppState::ColorSel colorFromString(const std::string&);
    
	// removed: star poly storage
	//bool testscale = false; 

	// Cumulative rotation angle in radians (wrapped to [0, 2*pi))
	float rotationAngleRad = 0.0f;
	// Cumulative wave phase for StaticWave (wrapped to [0, 2*pi))
	float wavePhaseRad = 0.0f;
	// Cumulative movement phase for Move LFO (wrapped to [0, 2*pi))
	float movePhaseRad = 0.0f;
	// Non-wrapping movement time in cycles for Random movement (ensures no repetition)
	double moveTimeCycles = 0.0;
	// Preserve last applied movement offset so that when moveSpeed becomes 0 the shape "stalls" at
	// its current displaced location instead of snapping back to the base manual position.
	ofVec2f lastMoveOffset {0.0f, 0.0f};
	// Per-run random seeds for Random movement noise space
	float randomSeedX = 0.0f;
	float randomSeedY = 0.0f;
	// Cumulative rainbow hue phase [0, 2*pi)
	float rainbowPhaseRad = 0.0f;
	// (strobe removed; scanrate now controls PPS override)
	// Until a scale OSC value is received (or a cue applied), use default unscaled = 1.0
	bool hasScaleInput = false;

	// Smooth scanrate (PPS) ramping to avoid DAC overload when jumping quickly.
	// When an OSC /laser/scanrate arrives we store target in state->scanRateHz, but also
	// keep an internal current value we increment toward target at a limited slew rate.
	int ppsCurrent = 0;          // last applied PPS (0 = disabled)
	int ppsTarget = 0;           // desired PPS target from OSC (0 = disabled)
	int ppsSlewPerFrame = 1200;  // (deprecated) legacy frame-based slew still used as fallback cap
	int ppsSlewPerSecond = 10000; // NEW: max PPS change per second (time-based ramp). 10k/sec keeps ~2s 0->max.
	// Alternative: time‑based ramp; we can later convert to per-second if needed.

	// --- Momentary cue support ---
	// While a /cue/momentary/{n} is held (value>0) we apply cue n then revert when released (value==0).
	bool momentaryCueActive = false;
	int activeMomentaryCueIndex = 0;
	CueState momentaryPrevState; // snapshot before applying momentary
	float prevRotationAngleRad = 0.0f;
	float prevWavePhaseRad = 0.0f;
	float prevMovePhaseRad = 0.0f;
	double prevMoveTimeCycles = 0.0;

	// --- Joystick button -> cue learn/mapping ---
	// Support mapping joystick buttons to cues using the same /learn workflow used for MIDI.
	// When in learn mode and a cue index is selected, the next joystick button PRESS will
	// be stored as a momentary mapping: button press sends /cue/momentary/{n} 1, release sends 0.
	struct JoystickCueMap { int joystick = -1; int button = -1; int cueIndex = 0; bool momentary = true; };
	std::vector<JoystickCueMap> joystickCueMaps; // loaded/saved from JSON
	std::unordered_map<uint32_t,bool> joyPrevButtonState; // key = (joystick<<16)|button
#if HAVE_OFXJOYSTICK
	ofxJoystick ofxJoy; // if addon present
#endif
	void loadJoystickCueMappings();
	void saveJoystickCueMappings();
	void addOrUpdateMomentaryCueJoystickMapping(int joystick, int button, int cueIndex);
	void pollJoysticksForLearningAndTriggers();
	ofxOscSender joystickOscSender; // local loopback sender (127.0.0.1:oscPort)
    
};

