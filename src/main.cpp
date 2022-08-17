#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

    ofGLWindowSettings settings;
    //settings.setGLVersion(3, 2);
   	settings.setSize(1280, 960);
   	settings.setGLVersion(3,2);
   	ofCreateWindow(settings);
   	ofSetWindowTitle("LAWKI-Passage");
	ofRunApp(new ofApp());

}
