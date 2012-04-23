#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"

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
    
	bool bDrawPointCloud;
	
	int angle;
    
    int threshold;
	
	// used for viewing the point cloud
	ofEasyCam easyCam;
    
    // audio from microphone
    void audioReceived (float * input, int bufferSize, int nChannels);
    float maxLevel;
};
