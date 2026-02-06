#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main(int argc, char *argv[]){
	// Parse command-line arguments
	bool headlessMode = false;
	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "--headless" || arg == "-h") {
			headlessMode = true;
			ofLogNotice() << "Starting BeamCommander in headless mode";
		} else if (arg == "--help") {
			std::cout << "BeamCommander - Laser Control System" << std::endl;
			std::cout << "Usage: " << argv[0] << " [OPTIONS]" << std::endl;
			std::cout << "Options:" << std::endl;
			std::cout << "  --headless, -h    Run in headless mode without UI" << std::endl;
			std::cout << "  --help            Show this help message" << std::endl;
			std::cout << std::endl;
			std::cout << "Headless Mode:" << std::endl;
			std::cout << "  When running headless, the application will:" << std::endl;
			std::cout << "  - Run without a window/UI" << std::endl;
			std::cout << "  - Use preferences from existing config files" << std::endl;
			std::cout << "  - Listen for OSC commands on port 9000" << std::endl;
			std::cout << "  - Serve 3D preview via HTTP on port 8080" << std::endl;
			std::cout << "  - Access preview at: http://localhost:8080" << std::endl;
			return 0;
		}
	}

	if (headlessMode) {
		// Headless mode: no window, console only
		ofGLFWWindowSettings settings;
		settings.setSize(800, 800);  // Virtual canvas size for rendering
		settings.setGLVersion(3, 2);
		ofCreateWindow(settings);
		ofSetWindowTitle("BeamCommander - Headless Mode");
		ofLogNotice() << "BeamCommander running in headless mode";
		ofLogNotice() << "OSC listening on port 9000";
		ofLogNotice() << "Web preview available at http://localhost:8080";
	} else {
		// Normal mode: windowed UI
		ofSetupOpenGL(1400, 980, OF_WINDOW);
		ofSetWindowTitle("BeamCommander - by Oliver Byte");
	}

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

	return 0;
}
