
#include "ofApp.h"
#include "ofConstants.h"
#include "ofxJSON.h"
#include "stdlib.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetVerticalSync(true);

	debug = false;
	debug_draw = 0;
	lastOSC = " -- ";

	// Create video tiles
	ofxJSONElement result;

	ofFilePath fp;
	string tileSettings = fp.join(fp.getUserHomeDir(), "lawki_config.json");
	ofFile config_file(tileSettings);
	if (!config_file.exists()) {
		ofLog() << "cannot find " << tileSettings;
		tileSettings = "lawki_config.json";
	}
	ofLog() << tileSettings << " loading tile settings";

	bool parseSuccessful = result.open(tileSettings);
	ofColor highlight;

	if (parseSuccessful) {
		int num = 0;
		string video_root = result["video_root"].asString();
		ofLog() << "video_root: " << video_root;
		for (Json::ArrayIndex i = 0; i < result["tiles"].size(); i++) {
			VideoTile vt {
				num,
				result["tiles"][i]["x"].asInt(),
				result["tiles"][i]["y"].asInt(),
				result["tiles"][i]["width"].asInt(),
				result["tiles"][i]["height"].asInt(),
				video_root
			};

			Json::Value subsections = result["tiles"][i]["subsections"];
			if (subsections.size() > 0) {
				for (Json::ArrayIndex j = 0; j < subsections.size(); j++) {
					vt.addSubsection(
						subsections[j]["sx"].asFloat(),
						subsections[j]["sy"].asFloat(),
						subsections[j]["sw"].asFloat(),
						subsections[j]["sh"].asFloat(),
						subsections[j]["dx"].asFloat(),
						subsections[j]["dy"].asFloat(),
						subsections[j]["dw"].asFloat(),
						subsections[j]["dh"].asFloat(),
						subsections[j]["flipX"].asBool(),
						subsections[j]["flipY"].asBool());
				}
			}

			if (!result["tiles"][i]["show"].isNull())
				vt.show_main = result["tiles"][i]["show"].asBool();

			num++;
			tiles.push_back(vt);
		}

		highlight = ofColor(result["highlight"][0].asInt(), result["highlight"][1].asInt(), result["highlight"][2].asInt());

		recv_port = result["port"].asInt();
		audio_port = result["audio_port"].asInt();
		sensor_map = result["sensor_map"];

		screenmap.load(result["screenmap"].asString());
	} else {
		ofLogError("cannot parse `tiles.json` config file");
		recv_port = 1881;
		audio_port = 1999;
		highlight = ofColor(155, 0, 255);
	}

	// Setup OSC listening
	ofLog() << "listening for OSC messages on port " + ofToString(recv_port);
	receiver.setup(recv_port);

	// Setup OSC sending
	ofLog() << "sending OSC messages to port localhost:" + ofToString(audio_port);
	sender.setup("localhost", audio_port);

	// Setup GUI controls
	gui.setup();
	gui.add(seed.set("seed", 1.0, 0, 1));
	gui.add(threshold.set("threshold", 0.3, 0, 3));
	gui.add(reaction.set("reaction", 0.7, 0, 1));
	gui.add(decay.set("decay", 0.7, 0, 1));
	gui.add(react.set("react", true));
	gui.add(highlightSlider.setup("highlight", highlight, ofColor(0), ofColor(255)));
	gui.add(centerSlider.setup("color", center, ofColor(0), ofColor(255)));
	//center.addListener(this, &ofApp::colorChanged);

	ofSetLogLevel(OF_LOG_WARNING);
}

//--------------------------------------------------------------
void ofApp::update() {

	// check for server messsages
	while (receiver.hasWaitingMessages()) {
		ofxOscMessage m;
		receiver.getNextMessage(m);

		// message expected to be
		// - "/video" {int: tile_number} {string: fp}
		// - "/sensor" {int: tile_number} {int: val}

		lastOSC = m.getAddress();
		for (size_t i = 0; i < m.getNumArgs(); i++) {
			lastOSC += " (" + m.getArgTypeName(i) + ") ";

			if (m.getArgType(i) == OFXOSC_TYPE_INT32 || m.getArgType(i) == OFXOSC_TYPE_INT64)
				lastOSC += ofToString(m.getArgAsInt(i));
			else if (m.getArgType(i) == OFXOSC_TYPE_FLOAT)
				lastOSC += ofToString(m.getArgAsFloat(i));
			else if (m.getArgType(i) == OFXOSC_TYPE_STRING)
				lastOSC += m.getArgAsString(i);
			else
				lastOSC += "<unknown>";
		}

		if (m.getAddress() == "/video") {
			int tile_number = m.getArgAsInt32(0);
			string fp = m.getArgAsString(1);

			if ((size_t)tile_number < tiles.size()) {
				for (size_t i = 0; i < tiles.size(); i++)
					tiles[i].setVolume(0.0);

				tiles[tile_number].setVideo(fp);
				tiles[tile_number].setVolume(1.0);
			}

		} else if (m.getAddress() == "/sensor") {
			int sensor_number = m.getArgAsInt32(0);
			int val = m.getArgAsInt32(1);

			newSensorValue(sensor_number, val);
		} else if (m.getAddress() == "/debug") {
			debug = !debug;
		} else {
			ofLog(OF_LOG_WARNING) << "OSC: " << m.getAddress() << " [unknown]";
		}
	}

	for (size_t i = 0; i < tiles.size(); i++)
		tiles[i].update(seed.get(), threshold.get(), reaction.get(), decay.get(), react.get(), highlightSlider, centerSlider);
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofBackground(highlightSlider);

	for (size_t i = 0; i < tiles.size(); i++)
		tiles[i].draw(debug_draw = debug_draw * (int)debug);

	if (debug) {
		if (debug_draw == 5 && screenmap.isAllocated())
			screenmap.draw(0, 0);

		gui.draw();
		ofSetColor(255);
		string info = "";
		switch (debug_draw) {
		case 0:
			info += "View 0: FINAL OUTPUT";
			break;
		case 1:
			info += "View 1: TILE INFO";
			break;
		case 2:
			info += "View 2: RAW VIDEO STREAM";
			break;
		case 3:
			info += "View 3: HIGHLIGHTED PIXELS";
			break;
		case 4:
			info += "View 4: REACTION RESULT";
			break;
		case 5:
			info += "View 5: TEST IMAGE";
			break;
		}
		info += "\n";
		info += ("FPS: " + ofToString(ofGetFrameRate(), 2) + "\n");
		info += "Listening on port " + ofToString(recv_port) + "\n";
		info += "Last OSC: " + lastOSC + "\n";
		info += "Drawing level: " + ofToString(debug_draw) + "\n";
		info += "Sensors: ";
		for (size_t i = 0; i < tiles.size(); i++)
			info += ofToString((int)tiles[i].active) + " ";

		ofDrawBitmapStringHighlight(info, 10, ofGetHeight() - 10 - 6 * 12);

		info = "";
		info += "Reaction Parameters:\n";
		info += " - seed: " + ofToString(seed.get(), 2) + "\n";
		info += " - threshold: " + ofToString(threshold.get(), 2) + "\n";
		info += " - reaction: " + ofToString(reaction.get(), 2) + "\n";
		info += " - decay: " + ofToString(decay.get(), 2) + "\n";
		info += " - react: " + ofToString(react.get(), 2);

		ofDrawBitmapStringHighlight(info, ofGetWidth() - 200, ofGetHeight() - 10 - 6 * 12);
	}
}

//--------------------------------------------------------------
void ofApp::exit() {
	for (size_t i = 0; i < tiles.size(); i++)
		tiles[i].close();
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	switch (key) {
	case 'q':
		ofExit(0);
		break;
	case 'd':
		debug = !debug;
		break;
	case '0':
	case '`':
		debug_draw = 0;
		break;
	case '1':
		debug_draw = 1;
		break;
	case '2':
		debug_draw = 2;
		break;
	case '3':
		debug_draw = 3;
		break;
	case '4':
		debug_draw = 4;
		break;
	case '5':
		debug_draw = 5;
		break;
	case 'z':
		newSensorValue(0, 1);
		break;
	case 'x':
		newSensorValue(1, 1);
		break;
	case 'c':
		newSensorValue(2, 1);
		break;
	case 'v':
		newSensorValue(3, 1);
		break;
	case 'b':
		newSensorValue(4, 1);
		break;
	case 'Z':
		newSensorValue(0, 0);
		break;
	case 'X':
		newSensorValue(1, 0);
		break;
	case 'C':
		newSensorValue(2, 0);
		break;
	case 'V':
		newSensorValue(3, 0);
		break;
	case 'B':
		newSensorValue(4, 0);
		break;
	case ' ':
		for (size_t i = 0; i < tiles.size(); i++)
			tiles[i].togglePause();
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::newSensorValue(int sensor_number, int val) {

	if ((size_t)sensor_number >= sensor_map.size())
		return;

	for (Json::Value::ArrayIndex i = 0; i < sensor_map[sensor_number].size(); i++) {
		int tile_number = sensor_map[sensor_number][i].asInt();
		tiles[tile_number].active = (bool)val;
	}

	ofxOscMessage m;
	m.setAddress("/sensor/" + ofToString(sensor_number));
	m.addInt32Arg(val);
	sender.sendMessage(m, false);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
}

//--------------------------------------------------------------
//--------------------------------------------------------------

VideoTile::VideoTile(int num, int x, int y, int width, int height, string video_root) {
	number = num;
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;

	m_seed = 1.0;
	m_threshold = 0.3;
	m_decay = 0.7;
	m_reaction = 0.7;
	m_react = true;

	m_video_root = video_root;

	show_main = true;

	c = 0;
	paused = false;
	active = true;
	currentVideo = "<default>";

	// Setup video player
	// player.load("movies/news.mp4");
	player.load("movies/lawki_logo.mp4");
	player.setLoopState(OF_LOOP_NORMAL);
	player.play();
	videoInitialising = false;

	// Visual shader setup
	shader.load("shaders/shader");

	// Seed shader setup
	seedShader.load("shaders/shader.vert", "shaders/seed.frag");
	seedFbo.allocate(width, height);

	// Video quad
	fbo.allocate(width, height);

	plane.set(width, height);
	plane.setScale(1, -1, 1);
	plane.setPosition(plane.getWidth() / 2, plane.getHeight() / 2, 0);
	plane.mapTexCoords(0, 0, player.getWidth(), player.getHeight());

	// Compute shader setup
	computeShader.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/computeshader.cs");
	computeShader.linkProgram();

	reactionTexture.allocate(width, height, GL_RGBA8);
	reactionTexture.bindAsImage(2, GL_WRITE_ONLY);

	// Bind simulation buffers
	A1.allocate(width * height * sizeof(float), A1cpu, GL_STATIC_DRAW);
	A2.allocate(width * height * sizeof(float), A2cpu, GL_STATIC_DRAW);

	A1.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	A2.bindBase(GL_SHADER_STORAGE_BUFFER, 1);
}

void VideoTile::update(float seed, float threshold, float reaction, float decay, bool react, ofColor color, ofColor center) {
	if (player.isLoaded()) {
		if (player.getWidth() > 0 && videoInitialising) {
			// video has loaded, update rendering plane size
			videoInitialising = false;
			player.play();
			plane.mapTexCoords(0, 0, player.getWidth(), player.getHeight());
		}

		player.update();
		currentFrame = player.getTexture();
	}

	m_color = color;

	seedFbo.begin();
	seedShader.begin();
	seedShader.setUniformTexture("video", currentFrame, 1);
	seedShader.setUniform1f("threshold", threshold);
	seedShader.setUniform3f("center", center.r / 255.0, center.g / 255, center.b / 255);
	plane.draw();
	seedShader.end();
	seedFbo.end();

	// Update reaction simulation
	c = 1 - c;

	A1.bindBase(GL_SHADER_STORAGE_BUFFER, c);
	A2.bindBase(GL_SHADER_STORAGE_BUFFER, 1 - c);

	reactionTexture.bindAsImage(2, GL_WRITE_ONLY);

	computeShader.begin();

	computeShader.setUniform1f("seed", seed);
	if (active) {
		computeShader.setUniform1f("decay", decay);
	} else {
		computeShader.setUniform1f("decay", 1.0);
	}
	computeShader.setUniform1f("reaction", reaction);
	computeShader.setUniform1f("elapsedTime", ofGetElapsedTimef());
	computeShader.setUniform2i("resolution", m_width, m_height);
	computeShader.setUniform1i("react", int(react));
	computeShader.setUniformTexture("seedSource", seedFbo.getTexture(), 3);

	computeShader.dispatchCompute(m_width / 8, m_height / 8, 1);
	computeShader.end();
}

void VideoTile::setVideo(string fp) {
	//player.close();
	try {
		player.loadAsync(m_video_root + fp);
		videoInitialising = true;
		currentVideo = fp;
	} catch (...) {
		ofLog(OF_LOG_ERROR) << "error loading video " + fp;
		player.loadAsync("movies/lawki_logo.mp4");
		videoInitialising = true;
		currentVideo = fp;
	}
}

void VideoTile::addSubsection(SubSection s) {
	subsections.push_back(s);
	ofLog() << "SubSection:";
	ofLog() << "   sx " << s.sx << ", sy " << s.sy << ", sw " << s.sw << ", sh " << s.sh;
	ofLog() << "   dx " << s.dx << ", dy " << s.dy << ", dw " << s.dw << ", dh " << s.dh;
}

void VideoTile::addSubsection(float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, bool flipX, bool flipY) {
	addSubsection({ sx, sy, sw, sh, dx, dy, dw, dh, flipX, flipY });
}

void VideoTile::draw(int debug_draw = 0) {
	fbo.begin();
	shader.begin();
	shader.setUniformTexture("video", currentFrame, 1);
	shader.setUniformTexture("reaction", reactionTexture, 2);
	shader.setUniform2i("videoResolution", player.getWidth(), player.getHeight());
	shader.setUniform2i("reactionResolution", m_width, m_height);
	shader.setUniform3i("color", m_color.r, m_color.g, m_color.b);
	plane.draw();
	shader.end();
	fbo.end();

	ofTexture & fboTexture = fbo.getTexture();

	switch (debug_draw) {
	case 0:
		if (show_main)
			fbo.draw(m_x, m_y, m_width, m_height);

		for (SubSection & s : subsections) {
			float dx = s.dx + m_x;
			float dy = s.dy + m_y;
			float dw = s.dw;
			float dh = s.dh;

			if (s.flipX) {
				dx += dw;
				dw *= -1;
			}

			if (s.flipY) {
				dy += dh;
				dh *= -1;
			}

			fboTexture.drawSubsection(dx, dy, dw, dh, s.sx, s.sy, s.sw, s.sh);
		}
		break;
	case 1: {
		ofFill();
		ofSetColor(120);
		ofDrawRectangle(m_x, m_y, m_width, m_height);
		ofNoFill();
		ofSetColor(0);
		ofDrawRectangle(m_x, m_y, m_width, m_height);
		ofDrawLine(m_x, m_y, m_x + m_width, m_y + m_height);
		ofDrawLine(m_x, m_y + m_height, m_x + m_width, m_y);

		for (size_t i = 0; i < subsections.size(); i++) {
			ofSetColor(255, 0, 0);
			ofDrawRectangle(subsections[i].dx + m_x, subsections[i].dy + m_y, subsections[i].dw, subsections[i].dh);
			ofDrawBitmapString("destination", subsections[i].dx + m_x, subsections[i].dy + m_y - 3);

			float tx = subsections[i].dx + m_x + 5;
			float ty = subsections[i].dy + m_y + 5;

			if (subsections[i].flipX)
				tx += subsections[i].dw - 10;

			if (subsections[i].flipY)
				ty += subsections[i].dh - 10;

			ofFill();
			ofDrawCircle(tx, ty, 3);
			ofNoFill();

			ofSetColor(0, 0, 255);
			ofDrawRectangle(subsections[i].sx + m_x, subsections[i].sy + m_y, subsections[i].sw, subsections[i].sh);
			ofDrawBitmapString("source", subsections[i].sx + m_x, subsections[i].sy + m_y - 3);

			ofFill();
			ofDrawCircle(subsections[i].sx + m_x + 5, subsections[i].sy + m_y + 5, 3);
			ofNoFill();
		}

		ofSetColor(0);
		string name;
		name = "Screen " + ofToString(number);
		ofDrawBitmapString(name, m_x + m_width / 2 - name.length() * 4, m_y + 14);
		name = "(" + ofToString(m_x) + ", " + ofToString(m_y) + ") " + ofToString(m_width) + "x" + ofToString(m_height);
		ofDrawBitmapString(name, m_x + m_width / 2 - name.length() * 4, m_y + 26);
		name = "current video: " + currentVideo;
		ofDrawBitmapString(name, m_x + m_width / 2 - name.length() * 4, m_y + 38);
		name = "paused: " + ofToString(player.isPaused());
		ofDrawBitmapString(name, m_x + m_width / 2 - name.length() * 4, m_y + 50);
		name = "active: " + ofToString(active);
		ofDrawBitmapString(name, m_x + m_width / 2 - name.length() * 4, m_y + 62);
		name = "show: " + ofToString(show_main);
		ofDrawBitmapString(name, m_x + m_width / 2 - name.length() * 4, m_y + 74);
		ofSetColor(255);
	} break;
	case 2:
		currentFrame.draw(m_x, m_y, m_width, m_height);
		break;
	case 3:
		seedFbo.draw(m_x, m_y);
		break;
	case 4:
		reactionTexture.draw(m_x, m_y);
		break;
	}
}

void VideoTile::close() {
	player.close();
}

void VideoTile::setVolume(float vol) {
	player.setVolume(vol);
}

void VideoTile::togglePause() {
	if (player.isPaused()) {
		player.setPaused(false);
	} else {
		player.setPaused(true);
	}
}
