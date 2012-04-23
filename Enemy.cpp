//
//  Enemy.cpp
//  ofxKinectExample
//
//  Created by Jane Friedhoff on 4/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "Enemy.h"
#include "ofMain.h"

void Enemy::setup(float x, float y) {
    xPos = x;
    yPos = y;
    
    ySpeed = 2;

    isAlive = true;
    
    dimension = ofRandom(1, 100000);
    radian = 0;
}

void Enemy::update() {
    yPos+=ySpeed;    
    radian+=.05; // may want to lower this--could be faster than people can run/anticipate
    xPos += 2*sin(radian/2); // using sin for more graceful back and forth movement
    
}

void Enemy::draw() {
    ofFill();
    ofSetColor(0, 206, ofRandom(160, 240));
    ofRect(xPos, yPos, 30, 30);
}