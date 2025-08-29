#include "ofApp.h"
#include "ofxOsc.h"
#include "ofxLaser.h"
#include "ofJson.h" // inline comment: nlohmann::json via ofJson

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(false);

    // Laser: nichts weiter nötig – ofxLaser initialisiert sich selbst
    // Optional: laser.setCanvasSize(800,800);

    // OSC
    osc.setup(9000); // inline comment: listen on UDP/9000
}

//--------------------------------------------------------------
void ofApp::update(){
    handleOsc();
    laser.update(); // inline comment: prepare laser manager
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(5,5,10);

    // inline comment: draw received polygon (if available)
    if (hasShape && !polyPoints.empty()){
        ofPolyline p = buildPoly();
        laser.drawPoly(p, color, profile);
    }

    laser.send();   // inline comment: push frame to DAC
    laser.drawUI(); // inline comment: on-screen preview/UI
}

//--------------------------------------------------------------
ofPolyline ofApp::buildPoly() const {
    // inline comment: apply TR (rotation around local center happens client-side or here via matrix if needed)
    ofPolyline p;
    // rotate around origin, then translate
    float rad = ofDegToRad(rotDeg);
    glm::mat2 R = glm::mat2(cos(rad), -sin(rad),
                            sin(rad),  cos(rad));
    for (auto &v : polyPoints){
        glm::vec2 r = R * v + pos;
        p.addVertex(r);
    }
    p.setClosed(true);
    return p;
}

//--------------------------------------------------------------
void ofApp::handleOsc(){
    ofxOscMessage m;
    while (osc.hasWaitingMessages()){
        osc.getNextMessage(m);
        if (m.getAddress() == "/laser/shape"){
            try{
                // inline comment: JSON payload is first arg (string)
                ofJson j = ofJson::parse(m.getArgAsString(0));

                // schema:
                // {
                //   "points":[[x,y],...], "pos":[x,y], "rot":deg,
                //   "color":[r,g,b], "profile":"fast|default|detail"
                // }

                // points
                vector<glm::vec2> pts;
                for (auto &pt : j["points"]) pts.emplace_back(pt[0], pt[1]);

                // transform & style
                glm::vec2 p = { j.value("pos", ofJson::array({400,300}))[0],
                                j.value("pos", ofJson::array({400,300}))[1] };
                float rot = j.value("rot", 0.0f);

                ofColor c = ofColor::white;
                if (j.contains("color") && j["color"].is_array()){
                    c = ofColor( (int)j["color"][0], (int)j["color"][1], (int)j["color"][2] );
                }

                ofxLaser::RenderProfile prof = OFXLASER_PROFILE_DEFAULT;
                if (j.value("profile", std::string("default")) == "fast")   prof = OFXLASER_PROFILE_FAST;
                if (j.value("profile", std::string("default")) == "detail") prof = OFXLASER_PROFILE_DETAIL;

                // commit
                polyPoints = std::move(pts);
                pos = p;
                rotDeg = rot;
                color = c;
                profile = prof;
                hasShape = true;
            } catch (std::exception& e){
                ofLogError() << "OSC/JSON parse error: " << e.what();
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs& e){
    if(e.key==OF_KEY_TAB) laser.selectNextLaser();
}
