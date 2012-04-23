//
//  Player.cpp
//  ofxKinectExample
//
//  Created by Jane Friedhoff on 4/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "ofMain.h"
#include "Player.h"

void Player::setup() {
    xPos = 0;
    yPos = ofGetHeight();
    
    height = 20;
    width = 20;
    
    currentTime = 0;
    startTime = ofGetElapsedTimeMillis();
    totalTime = 1000;
    
    hasStartedYelling = false;
    isWinning = false;
    
    numKilled = 0;
}

void Player::update() {

}

void Player::draw() {
    
    ofFill();
    if (isWinning) {
        ofSetColor(ofRandom(255), ofRandom(255), ofRandom(255));
    } else {
    ofSetColor(199, 21, 133);
    }
    ofRect(xPos, ofGetHeight()-20, 30, 20);
}

void Player::shoot() {
    
}


