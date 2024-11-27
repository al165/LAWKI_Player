#include "ofApp.h"
#include "ofMain.h"
#include "ofxJSON.h"

//========================================================================
int main() {

	ofxJSONElement result;
	ofxJSONElement windowSettings;

	ofFilePath fp;
	string tileSettings = fp.join(fp.getUserHomeDir(), "lawki_config.json");
	ofFile config_file(tileSettings);
	if (!config_file.exists()) {
		ofLog() << "cannot find " << tileSettings;
		tileSettings = "lawki_config.json";
	}
	ofLog() << tileSettings << " loading tile settings";
	bool parseSuccessful = result.open(tileSettings);

	if (parseSuccessful)
		windowSettings = result["window"];
	else {
		ofLogError("cannot parse `tiles.json` config file");
		return 1;
	}

	ofGLFWWindowSettings settings;

	settings.setSize(windowSettings["width"].asInt(), windowSettings["height"].asInt());
	settings.setGLVersion(3, 2);

	settings.decorated = false;
	settings.resizable = false;

	if (result["fullscreen"].asBool()) {
		ofLog() << "Fullscreen mode";
		settings.windowMode = OF_FULLSCREEN;
	} else {
		ofLog() << "Windowed mode";
		settings.windowMode = OF_WINDOW;
	}

	settings.setPosition(ofVec2f(windowSettings["x"].asInt(), windowSettings["y"].asInt()));

	ofCreateWindow(settings);
	ofSetWindowTitle("LAWKI - ARK");

	ofRunApp(new ofApp());
}
