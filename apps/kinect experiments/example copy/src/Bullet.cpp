//
//  Bullet.cpp
//  ofxKinectExample
//
//  Created by Jane Friedhoff on 4/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "ofMain.h"
#include "Bullet.h"

void Bullet::setup(float r, float x) {
    xPos = x;
    yPos = ofGetHeight();
    
    yVelocity = 25;
    radius = r;
}

void Bullet::update() {
    yPos -= yVelocity;
}

void Bullet::draw() {
    ofFill();
    ofSetColor(255, 0, 0);
    ofEllipse(xPos, yPos, radius, radius);
}
