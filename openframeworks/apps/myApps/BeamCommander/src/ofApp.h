#pragma once
#include "ofMain.h"
#include "ofxLaserManager.h"
#include "ofxOsc.h"
#include "AppState.h"
#include <array>
#include "MidiToOscMapper.h"

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
    void keyPressed(ofKeyEventArgs& e);
    void updateOsc();

    ofxLaser::Manager laser;
    std::shared_ptr<AppState> state = std::make_shared<AppState>();

    // OSC
    ofxOscReceiver osc; int oscPort = 9000; void updateOscLegacy();

    std::unique_ptr<MidiToOscMapper> midiMapper;

    std::atomic<bool> learnArmed{false};
    std::atomic<int> learnCueIndex{0};
    std::atomic<bool> learnMomentaryArmed{false};
    std::atomic<int> learnMomentaryCueIndex{0};

    bool flashActive=false; float flashPrevBrightness=0.0f; std::atomic<int> flashReleaseMs{150}; bool flashDecaying=false; uint64_t flashDecayStartMs=0; float flashDecayFrom=1.0f;

    struct CueState { AppState::Shape shape; AppState::ColorSel colorSel; AppState::Movement movement; AppState::BeamFx beamFx; bool useCustom; float r,g,b; float rainbowSpeed,rainbowAmount,rainbowBlend; float waveFrequency,waveAmplitude,waveSpeed; float moveSpeed,moveSize; float rotationSpeed; float shapeScale; float posX,posY; float dotAmount; float scanRateHz; bool populated=false; };
    std::array<CueState,30> cues; bool saveArmed=false; void snapshotToCue(int idx); bool applyCue(int idx);

    const std::string cuesFileName = "cues.json"; void saveCuesToDisk(); void loadCuesFromDisk();
    static std::string shapeToString(AppState::Shape s); static AppState::Shape shapeFromString(const std::string&); static std::string colorToString(AppState::ColorSel c); static AppState::ColorSel colorFromString(const std::string&);

    float rotationAngleRad=0.0f; float wavePhaseRad=0.0f; float movePhaseRad=0.0f; double moveTimeCycles=0.0; ofVec2f lastMoveOffset{0,0}; float randomSeedX=0.0f, randomSeedY=0.0f; float rainbowPhaseRad=0.0f; bool hasScaleInput=false;

    int ppsCurrent=0; int ppsTarget=0; int ppsSlewPerFrame=1200; int ppsSlewPerSecond=10000;

    bool momentaryCueActive=false; int activeMomentaryCueIndex=0; CueState momentaryPrevState; float prevRotationAngleRad=0.0f; float prevWavePhaseRad=0.0f; float prevMovePhaseRad=0.0f; double prevMoveTimeCycles=0.0;

    struct JoystickCueMap { int joystick=-1; int button=-1; int cueIndex=0; bool momentary=true; };
    std::vector<JoystickCueMap> joystickCueMaps; std::unordered_map<uint32_t,bool> joyPrevButtonState; 
#if HAVE_OFXJOYSTICK
    ofxJoystick ofxJoy;
#endif
    void loadJoystickCueMappings(); void saveJoystickCueMappings(); void addOrUpdateMomentaryCueJoystickMapping(int joystick,int button,int cueIndex); void pollJoysticksForLearningAndTriggers(); ofxOscSender joystickOscSender;
};
