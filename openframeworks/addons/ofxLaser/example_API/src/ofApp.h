#pragma once
#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxLaser.h"

class ofApp : public ofBaseApp {
public:
    void setup() override;
    void update() override;
    void draw() override;
    void keyPressed(ofKeyEventArgs& e) override;

private:
    // --- Laser ---
    ofxLaser::Manager laser;

    // --- OSC ---
    ofxOscReceiver osc; // inline comment: OSC receiver

    // --- Runtime shape (simple demo schema) ---
    // inline comment: One polygon + transform + color + profile
    vector<glm::vec2> polyPoints;
    glm::vec2 pos = {400, 300};
    float rotDeg = 0.f;
    ofColor color = ofColor::white;
    ofxLaser::RenderProfile profile = OFXLASER_PROFILE_DEFAULT;
    bool hasShape = false;

    // util
    ofPolyline buildPoly() const;
    void handleOsc();
};
