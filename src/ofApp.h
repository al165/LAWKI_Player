#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOsc.h"
#include "ofxJSON.h"

#define W 1028
#define H 768

class VideoTile {
	public:
		VideoTile(int num, int x, int y, int width, int height);
		void update(float seed, float threshold, float reaction, float decay, bool react, ofColor color, ofColor center);
		void draw(int debug_draw);
		void close();
		void setVideo(string fp);
		void setVolume(float vol);
		void togglePause();

    int number;
		float m_seed, m_threshold, m_decay, m_reaction;
		bool m_react;
		ofColor m_color;

		bool paused;
		bool active;
		string currentVideo;

	private:
		int m_x, m_y;
		int m_width, m_height;

		ofBufferObject A1, A2;
		ofVideoPlayer player;
		ofTexture currentFrame;
		bool videoInitialising;
		ofShader shader, computeShader, seedShader;
		ofTexture reactionTexture;
		ofPlanePrimitive plane;
		ofFbo fbo, seedFbo;

		float A1cpu[W*H];
		float A2cpu[W*H];

		int c;
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

        bool debug;
        int debug_draw;

		vector<VideoTile> tiles;

        ofxOscSender sender;
		ofxOscReceiver receiver;
		string lastOSC;

		Json::Value sensor_map;

		ofColor highlight, center;
		int recv_port;
		int audio_port;

		ofxPanel gui;
		ofParameter<float> seed, threshold, decay, reaction;
		ofParameter<bool> react;
		ofxColorSlider centerSlider, highlightSlider;
};


