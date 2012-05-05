//
//  Boss.h
//  ofxKinectExample
//
//  Created by Jane Friedhoff on 5/4/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ofxKinectExample_Boss_h
#define ofxKinectExample_Boss_h
#include "ofMain.h"

class Boss {
public:
    float xPos;
    float yPos;
    
    float ySpeed;
    float xSpeed;
    
    ofColor color;
    
    int health;
    
    bool shieldsUp; // maybe use this, maybe not
    
    void setup();
    void update();
    void draw();
    void explode();
    
    ofTrueTypeFont font;
    
    bool bossOnScreen;
    
    int shieldTimer;
    int shieldTime;
    
    float radian;
    
    int explodeTimer;
    int explodeTime;
};

#endif
