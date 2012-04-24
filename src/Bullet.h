//
//  Bullet.h
//  ofxKinectExample
//
//  Created by Jane Friedhoff on 4/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ofxKinectExample_Bullet_h
#define ofxKinectExample_Bullet_h

class Bullet {
public:
    float xPos;
    float yPos;
    
    float xVelocity;
    float yVelocity;
    
    float radius;
    
    int colorH;
    
    ofColor color;
    
    int red;
    
    int difference;
    int playerF;
    
    void setup(float r, int f, int d, float x);
    void update();
    void draw();
    
};



#endif
