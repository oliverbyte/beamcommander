#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
    
    // NOTE that you no longer need to set up anything at all in ofxLaser!
    ofSetVerticalSync(false); 
   
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    // prepares laser manager to receive new graphics
    laser.update();
}


void ofApp::draw() {
    
    ofBackground(5,5,10);
    // Draw only a large blue circle in the center
    laser.drawCircle(400, 400, 200, ofColor(0, 50, 255), OFXLASER_PROFILE_FAST);
        
    // sends points to the DAC
    laser.send();
    // draw the laser UI elements
    laser.drawUI();
   
}



//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs& e){
    
    if(e.key==OF_KEY_TAB) {
        laser.selectNextLaser();
    }// if(e.key == ' ') {
     //   testscale = !testscale;
    //}
    
}
