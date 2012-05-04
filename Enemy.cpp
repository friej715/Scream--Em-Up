//
//  Enemy.cpp
//  ofxKinectExample
//
//  Created by Jane Friedhoff on 4/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "Enemy.h"

void Enemy::setup(float x, float y, string color) {
    xPos = x;
    yPos = y;
    
    ySpeed = 2;

    isAlive = true;
    
    dimension = ofRandom(1, 100000);
    radian = 0;
    
    enemyFont0.loadFont("invaders.ttf", 30);
    
    if (color == "blue") {
        eColor.setHsb(140, 255, 255, 255);
    } else if (color == "yellow") {
        eColor.setHsb(40, 255, 255, 255);
    } else if (color == "red") {
        eColor.setHsb(0, 255, 255, 255);
    }
    
    // assets
}

void Enemy::update() {
    yPos+=ySpeed;    
    radian+=.07; // may want to lower this--could be faster than people can run/anticipate
    xPos += 1.5*sin(radian/2); // using sin for more graceful back and forth movement
    
}

void Enemy::draw() {
    ofSetColor(eColor);
    ofFill();
    enemyFont0.drawString("B", xPos, yPos);
}