#pragma once

#include "ofMain.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Player.h"
#include "Boss.h"
#include "ofxOsc.h"
#include "ofxOpenNI.h"

//#define USE_IR // Uncomment this to use infra red instead of RGB cam...

// now we set the ports for our OSC messages
#define PORT1 8000 // player 1, 8000 for now
#define PORT2 8001 // player 2, 8001 for now


// uncomment this to read from two kinects simultaneously
//#define USE_TWO_KINECTS

class testApp : public ofBaseApp {
public:
	
	void setup();
	void update();
	void draw();
//	void exit();
	
	void drawPointCloud();
	
	void keyPressed(int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
    
    // game functions
    void addBullets();
    void checkBullets();
    void reset();

    // audio from microphone
    void audioReceived (float * input, int bufferSize, int nChannels);
    float * inputBufferCopy;
    
    float volumeP1;
    float maxLevelP1;
    vector<float> maxLevelsP1; // for smoothing
    
    float volumeP2;
    float maxLevelP2;
    vector<float> maxLevelsP2; // for smoothing

    
    int numLevelsToStore; // number of max levels to store
    
    // game!
    Enemy enemy;
    Player player1;
    Player player2;
    
    vector<Bullet> bulletsP1; // a vector of bullets so it can be dynamic
    vector<Bullet> bulletsP2;
    vector<Enemy> enemies;
    
    // vectors for data from text file re: unspawned enemies
    vector<float> enemyXPos;
    vector<float> enemyYPos;
    vector<float> enemyTimeSpawn;
    vector<string> enemyColors;
    
    void loadFromText();
    
    //timers
    int bulletTimerP1;
    int bulletTimeP1; //actual interval you want to use
    
    int bulletTimerP2;
    int bulletTimeP2; //actual interval you want to use
    
    int gameStartTime;
    
    float gameScale;
    
    int enemiesShot;
    
    int enemyTimer;
    int enemyTime;
    
    //debugging/testing
    float micSensitivity;
    bool grayImageOn;
    float maxLevel;
    
    bool isUsingKeyboard;
    
    // osc
    // okay, let's talk about this for a second. below you'll find one osc receiver.
    ofxOscReceiver receiverP1;
    ofxOscReceiver receiverP2;
    
    
    // the vector for the yelling, which will store all the volume variables of that yell.
    vector<float> newYellP1;
    vector<float> newYellP2;
    vector<float> oldYell;
    bool compareYells();
    void checkIfYelling();
    
    // babby's first fft variables. let's just go with one int (for the location of the max) rather than a vector. although we'll want a vector for point bonuses so we can smooth and see how long they're the same, probably.
    int p1MaxLocForFFT;
    int p2MaxLocForFFT;
    
    // visual effect tryout
    float shakeSensitivity;
    float maxShake;
    
    // gamestates, ughhhhh
    int gameState;
    
    // fonts
    ofTrueTypeFont titleFont;
    ofTrueTypeFont scoreFont;
    
    // logo
    ofImage logo;
    
    // background stars
    vector<ofVec2f> regularStars;
    
    Boss boss;
    
    // new kinect variables
    void	setupRecording(string _filename = "");
	void	setupPlayback(string _filename);
	string	generateFileName();
    
	bool				isLive, isTracking, isRecording, isCloud, isCPBkgnd, isMasking;
	bool				isTrackingHands, isFiltering;
    
	ofxOpenNIContext	recordContext, playContext;
	ofxDepthGenerator	recordDepth, playDepth;
    
#ifdef USE_IR
	ofxIRGenerator		recordImage, playImage;
#else
	ofxImageGenerator	recordImage, playImage;
#endif
    
	ofxHandGenerator	recordHandTracker, playHandTracker;
    
	ofxUserGenerator	recordUser, playUser;
	ofxOpenNIRecorder	oniRecorder;
    
#if defined (TARGET_OSX) //|| defined(TARGET_LINUX) // only working on Mac/Linux at the moment (but on Linux you need to run as sudo...)
	ofxHardwareDriver	hardware;
#endif
    
	void				drawMasks();
	void				drawPointCloud(ofxUserGenerator * user_generator, int userID);
    
	int					nearThreshold, farThreshold;
	int					pointCloudRotationY;
    
	ofImage				allUserMasks, user1Mask, user2Mask, depthRangeMask;
    
	float				filterFactor;
    
    int hipX1;
    
    int blinkCounter;

    vector<float> angles;
    float startAngle;
    
    ofSoundPlayer theme;

    
    
};
