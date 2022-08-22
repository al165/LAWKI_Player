#include "ofMain.h"
#include "ofApp.h"
#include "ofxJSON.h"

//========================================================================
int main( ){

    ofxJSONElement result;
    ofxJSONElement windowSetings;    
    string tileSettings = "tiles.json";
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
   	settings.windowMode = OF_WINDOW;
   	settings.setPosition(ofVec2f(windowSetings["x"].asInt(), windowSetings["y"].asInt()));

   	ofCreateWindow(settings);
   	ofSetWindowTitle("LAWKI-Passages");

    ofRunApp(new ofApp());

}
