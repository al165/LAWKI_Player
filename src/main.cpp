#include "ofMain.h"
#include "ofApp.h"
#include "ofxJSON.h"

//========================================================================
int main( ){

    ofxJSONElement result;
    ofxJSONElement windowSetings;    

    ofFilePath fp;
    string tileSettings = fp.join(fp.getUserHomeDir(), "lawki_config.json");
    ofFile config_file(tileSettings);
    if(!config_file.exists()){
        ofLog() << "cannot find " << tileSettings;
        tileSettings = "tiles.json";
    }
    ofLog() << tileSettings << " loading tile settings";
    bool parseSuccessful = result.open(tileSettings);

    if(parseSuccessful){
        windowSetings = result["window"];
    } else {
        ofLogError("cannot parse `tiles.json` config file");
        return 1;
    }

    ofGLFWWindowSettings settings;

   	settings.setSize(windowSetings["width"].asInt(), windowSetings["height"].asInt());
   	settings.setGLVersion(3,2);

   	settings.decorated = false;
   	settings.resizable = false;
   	settings.windowMode = OF_FULLSCREEN;
   	settings.setPosition(ofVec2f(windowSetings["x"].asInt(), windowSetings["y"].asInt()));

   	ofCreateWindow(settings);
   	ofSetWindowTitle("LAWKI-Passages");

    ofRunApp(new ofApp());

}
