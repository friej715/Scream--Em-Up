#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Player.h"
#include "ofxOsc.h"

// uncomment this to read from two kinects simultaneously
//#define USE_TWO_KINECTS

class testApp : public ofBaseApp {
public:
	
	void setup();
	void update();
	void draw();
	void exit();
	
	void drawPointCloud();
	
	void keyPressed(int key);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	ofxKinect kinect;
	
#ifdef USE_TWO_KINECTS
	ofxKinect kinect2;
#endif
	
	ofxCvColorImage colorImg;
	
	ofxCvGrayscaleImage grayImage; // grayscale depth image    
    ofxCvGrayscaleImage grayBg;
    ofxCvGrayscaleImage grayDiff;
	
	ofxCvContourFinder contourFinder;
	
    bool bLearnBackground;
	
	int angle;
    int threshold;
	
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
    
    // visual effect tryout
    float shakeSensitivity;
    float maxShake;
    
    // gamestates, ughhhhh
    int gameState;
};
