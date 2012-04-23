//
//  Player.h
//  ofxKinectExample
//
//  Created by Jane Friedhoff on 4/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ofxKinectExample_Player_h
#define ofxKinectExample_Player_h

class Player {
public:
    float xPos;
    float yPos;
    
    float width;
    float height;
    
    float xVelocity;
    
    float currentTime;
    float startTime;
    float totalTime;
    
    bool hasStartedYelling;
    bool isShooting;
    bool isWinning;
    
    void setup();
    void update();
    void draw();
    void shoot();
    
    int numKilled;
    
    
};

#endif
