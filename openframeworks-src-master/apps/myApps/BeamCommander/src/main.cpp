#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	ofSetupOpenGL(1400,980,OF_WINDOW);			// <-------- setup the GL context
	ofSetWindowTitle("BeamCommander - by Oliver Byte");

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
