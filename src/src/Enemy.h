//
//  Enemy.h
//  ofxKinectExample
//
//  Created by Jane Friedhoff on 4/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ofxKinectExample_Enemy_h
#define ofxKinectExample_Enemy_h
#include "ofMain.h"

class Enemy {
public:
    float xPos;
    float yPos;
    float xSpeed;
    float ySpeed;
    float xVelocity;
    float yVelocity;
    
    bool isAlive;
    
    float dimension; // for noise, in case it's used for randomish movement
    float radian; // related to noise movement?
    
    void setup(float x, float y, string s);
    void draw();
    void update();    
    
    //ofImage enemy1;
    
    ofTrueTypeFont enemyFont0;

    ofColor eColor;
};






#endif
