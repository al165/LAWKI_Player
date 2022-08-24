#include "stdlib.h"
#include "ofApp.h"
#include "ofConstants.h"
#include "ofxJSON.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);

    debug = false;
    debug_draw = 0;
    lastOSC = " -- ";

    // Create video tiles
    ofxJSONElement result;
    string tileSettings = "tiles.json";
    bool parseSuccessful = result.open(tileSettings);
    ofColor highlight;

    if(parseSuccessful){
        int num = 0;
        string video_root = result["video_root"].asString();
        ofLog() << "video_root: " << video_root;
        for(Json::ArrayIndex i=0; i < result["tiles"].size(); i++){
            VideoTile vt{
                num, 
                result["tiles"][i][0].asInt(), 
                result["tiles"][i][1].asInt(), 
                result["tiles"][i][2].asInt(), 
                result["tiles"][i][3].asInt(),
                video_root
            };
            num++;
            tiles.push_back(vt);   
        }

        highlight = ofColor(result["highlight"][0].asInt(), result["highlight"][1].asInt(), result["highlight"][2].asInt());

        recv_port = result["port"].asInt();
        audio_port = result["audio_port"].asInt();

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
    gui.add(center.setup("color", highlight, ofColor(0), ofColor(255)));
    //center.addListener(this, &ofApp::colorChanged);

    ofSetLogLevel(OF_LOG_WARNING);
}


//--------------------------------------------------------------
void ofApp::update(){

    // check for server messsages
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m;
        receiver.getNextMessage(m);

        // OLD:
        // message expected to be of form "/station_{sn}/{type}/{tile_number}/"
        // where `type` is either "video", "audio", "sensor", "close".
        // 
        // NEW:
        // message expected to be 
        // - "/video" {int: tile_number} {string: fp}
        // - "/sensor" {int: tile_number} {int: val}

        lastOSC = m.getAddress();
        for(size_t i = 0; i < m.getNumArgs(); i++){
            lastOSC += " (" + m.getArgTypeName(i) + ") ";
            if(m.getArgType(i) == OFXOSC_TYPE_INT32 || m.getArgType(i) == OFXOSC_TYPE_INT64){
                lastOSC += ofToString(m.getArgAsInt(i));
            } else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
                lastOSC += ofToString(m.getArgAsFloat(i));
            } else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
                lastOSC += m.getArgAsString(i);
            } else {
                lastOSC += "<unknown>";
            }
        }

        if(m.getAddress() == "/video"){
            int tile_number = m.getArgAsInt32(0);
            string fp = m.getArgAsString(1);

    	    //ofSetLogLevel(OF_LOG_NOTICE);
            //ofLog() << "OSC: /video " << tile_number << " " << fp;
    	    //ofSetLogLevel(OF_LOG_WARNING);

            for(size_t i=0; i < tiles.size(); i++){
                tiles[i].setVolume(0.0);
            }

    	    if((size_t)tile_number < tiles.size()){
                tiles[tile_number].setVideo(fp);
                tiles[tile_number].setVolume(1.0);
    	    }

        } else if(m.getAddress() == "/sensor"){
            int tile_number = m.getArgAsInt32(0);
            int val = m.getArgAsInt32(1);

            tiles[tile_number].active = (bool)val;

            //ofSetLogLevel(OF_LOG_NOTICE);
            //ofLog() << "OSC: /sensor " << tile_number << " " << val;
            //ofSetLogLevel(OF_LOG_WARNING);

            // relay sensor data to sound server
            ofxOscMessage m;
            m.setAddress("/sensor/" + ofToString(tile_number));
            m.addInt32Arg(val);
            sender.sendMessage(m, false);

        } else {
    	    ofLog(OF_LOG_WARNING) << "OSC: " << m.getAddress() << " [unknown]";
        }
    }

    for(size_t i=0; i < tiles.size(); i++){
    	tiles[i].update(seed.get(), threshold.get(), reaction.get(), decay.get(), react.get(), center);
    }
}


//--------------------------------------------------------------
void ofApp::draw(){
    //ofBackground(highlight[0], highlight[1], highlight[2]);
    ofBackground(center);

    for (size_t i = 0; i < tiles.size(); i++) {
      tiles[i].draw(debug_draw = debug_draw * (int)debug);
    }

    if(debug){
        gui.draw();
        ofSetColor(255);
        string info = "";
        info += ("FPS: " + ofToString(ofGetFrameRate(), 2) + "\n");
        info += "Listening on port " + ofToString(recv_port) + "\n";
        info += "Last OSC: " + lastOSC + "\n";
        info += "Drawing level: " + ofToString(debug_draw) + "\n";
        info += "Sensors: ";
        for(size_t i=0; i < tiles.size(); i++){
        	info += ofToString((int)tiles[i].active) + " ";
        }

        ofDrawBitmapStringHighlight(info, 10, ofGetHeight() - 10 - 5*12);

        info = "Reaction Parameters:\n";
        info += " - seed: " + ofToString(seed.get(), 2) + "\n"; 
        info += " - threshold: " + ofToString(threshold.get(), 2) + "\n"; 
        info += " - reaction: " + ofToString(reaction.get(), 2) + "\n"; 
        info += " - decay: " + ofToString(decay.get(), 2) + "\n"; 
        info += " - react: " + ofToString(react.get(), 2);

        ofDrawBitmapStringHighlight(info, ofGetWidth() - 200, ofGetHeight() - 10 - 6*12);
    }
}

//--------------------------------------------------------------
void ofApp::exit(){
    for(size_t i=0; i < tiles.size(); i++){
    	tiles[i].close();
    }
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == 'd'){
        debug = !debug;
    } else if(key == '0' || key == '`'){
        debug_draw = 0;
    } else if(key == '1'){
        debug_draw = 1;
    } else if(key == '2'){
        debug_draw = 2;
    } else if(key == '3'){
        debug_draw = 3;
    } else if(key == '4'){
        debug_draw = 4;
    } else if(key == ' '){
        for(size_t i=0; i < tiles.size(); i++){
        	tiles[i].togglePause();
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}


//--------------------------------------------------------------
//--------------------------------------------------------------

VideoTile::VideoTile(int num, int x, int y, int width, int height, string video_root){
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

    c = 0;
    paused = false;
    active = true;
    currentVideo = "<default>";

    // Setup video player
    player.load("movies/lawki_logo.mp4");
    player.setLoopState(OF_LOOP_NORMAL);
    player.play();
    videoInitialising = false;

    // Visual shader setup
    //shader.load("shaders/test.vert", "shaders/test.frag");
    shader.load("shaders/shader");

    // Seed shader setup
    seedShader.load("shaders/shader.vert", "shaders/seed.frag");
    seedFbo.allocate(width, height);

    // Video quad
    fbo.allocate(width, height);

    plane.set(width, height);
    plane.setScale(1, -1, 1);
    plane.setPosition(plane.getWidth()/2, plane.getHeight()/2, 0);
    plane.mapTexCoords(0, 0, player.getWidth(), player.getHeight());

    // Compute shader setup
    computeShader.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/computeshader.cs");
    computeShader.linkProgram();

    reactionTexture.allocate(width, height, GL_RGBA8);
    reactionTexture.bindAsImage(2, GL_WRITE_ONLY);

    // Bind simulation buffers
    A1.allocate(width*height*sizeof(float), A1cpu, GL_STATIC_DRAW);
    A2.allocate(width*height*sizeof(float), A2cpu, GL_STATIC_DRAW);

    A1.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
    A2.bindBase(GL_SHADER_STORAGE_BUFFER, 1);
}

void VideoTile::update(
    float seed, 
    float threshold, 
    float reaction, 
    float decay, 
    bool react, 
    ofColor color){
    if(player.isLoaded()){
        if(player.getWidth() > 0 && videoInitialising){
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
    seedShader.setUniform3f("center", 1.0, 1.0, 1.0);
    plane.draw();
    seedShader.end();
    seedFbo.end();

    // Update reaction simulation
    c = 1-c;

    A1.bindBase(GL_SHADER_STORAGE_BUFFER, c);
    A2.bindBase(GL_SHADER_STORAGE_BUFFER, 1-c);

    reactionTexture.bindAsImage(2, GL_WRITE_ONLY);

    computeShader.begin();

    computeShader.setUniform1f("seed", seed);
    if(active){
        computeShader.setUniform1f("decay", decay);
    } else {
        computeShader.setUniform1f("decay", 1.0);
    }
    computeShader.setUniform1f("reaction", reaction);
    computeShader.setUniform1f("elapsedTime", ofGetElapsedTimef());
    computeShader.setUniform2i("resolution", m_width, m_height);
    computeShader.setUniform1i("react", int(react));
    computeShader.setUniformTexture("seedSource", seedFbo.getTexture(), 3);

    computeShader.dispatchCompute(m_width/20, m_height/20, 1);
    computeShader.end();
}

void VideoTile::setVideo(string fp){
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

void VideoTile::draw(int debug_draw=0){
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

    switch(debug_draw){
        case 0:
            fbo.draw(m_x, m_y, m_width, m_height);
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
            string name;
            name = "Screen " + ofToString(number);
            ofDrawBitmapString(name, m_x + m_width/2 - name.length()*4, m_y + 14);
            name = "(" + ofToString(m_x) + ", " + ofToString(m_y) + ") " + ofToString(m_width) + "x" + ofToString(m_height);
            ofDrawBitmapString(name, m_x + m_width/2 - name.length()*4, m_y + 26);
            name = "current video: " + currentVideo;
            ofDrawBitmapString(name, m_x + m_width/2 - name.length()*4, m_y + 38);
            name = "paused: " + ofToString(player.isPaused());
            ofDrawBitmapString(name, m_x + m_width/2 - name.length()*4, m_y + 50);
            name = "active: " + ofToString(active);
            ofDrawBitmapString(name, m_x + m_width/2 - name.length()*4, m_y + 62);
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

void VideoTile::close(){
    player.close();
}

void VideoTile::setVolume(float vol){
    player.setVolume(vol);
}

void VideoTile::togglePause(){
    if(player.isPaused()){
        player.setPaused(false);
    } else {
        player.setPaused(true);
    }
}
