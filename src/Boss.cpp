//
//  Boss.cpp
//  ofxKinectExample
//
//  Created by Jane Friedhoff on 5/4/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "Boss.h"

void Boss::setup() {
    xPos = ofGetWidth()/4;
    yPos = -100;
    
    ySpeed = 1;
    
    font.loadFont("invaders.ttf", 212);

    bossOnScreen = false;
    shieldsUp = false;
    
    shieldTimer = 0;
    shieldTime = 150;
    
    radian = 0;
    
    health = 100;
    
    explodeTimer = 0;
    explodeTime = 30;
}

void Boss::update() {
    if (yPos > 0) {
        shieldTimer++;
        
        radian+=.07; // may want to lower this--could be faster than people can run/anticipate
        xPos += 11*sin(radian/2); // using sin for more graceful back and forth movement
    }
    
    if (shieldTimer > shieldTime) {
        shieldTimer = 0;
        shieldsUp = !shieldsUp;
    }
    
    if (bossOnScreen == true && shieldsUp == false) {
        yPos += ySpeed;
        color.setHsb(ofRandom(255), 255, 255, 255);
    }

}

void Boss::draw() {
    
    if (shieldsUp == true) {
        ofSetColor(255, 155);
        ofCircle(xPos+20, yPos-120, 200);
    }
    
    if (bossOnScreen == true) {
        ofSetColor(color);
        font.drawString("F", xPos-150, yPos);
    }
    
    if (health <= 0) {
        bossOnScreen = false;
        explode();
    }
}

void Boss::explode() {
    explodeTimer++;
    if (explodeTimer > explodeTime && explodeTimer < 60) {
        font.drawString("F", xPos-150, yPos);
    }
}