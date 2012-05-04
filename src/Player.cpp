//
//  Player.cpp
//  ofxKinectExample
//
//  Created by Jane Friedhoff on 4/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "Player.h"

void Player::setup() {
    xPos = 0;
    yPos = ofGetHeight();
    
    height = 40;
    width = 20;
    
    currentTime = 0;
    startTime = ofGetElapsedTimeMillis();
    totalTime = 1000;
    
    hasStartedYelling = false;
    isWinning = false;
    
    numKilled = 0;
    
    playerFont.loadFont("arcade.ttf", 30);
    
}

void Player::update() {

}

void Player::draw() {
    ofFill();
    if (isWinning) {
        ofSetColor(ofRandom(255), ofRandom(255), ofRandom(255));
    } else {
    ofSetColor(0, 255, 255);
    }
    ofRect(xPos - 40, ofGetHeight()-20, 80, 30);
    ofRect(xPos - 30, ofGetHeight()-30, 60, 40);
    ofRect(xPos - 10, ofGetHeight()-40, 20, 30);
    ofRect(xPos - 5, ofGetHeight()-50, 10, 20);
    //playerFont.drawString("W", xPos, yPos-20);
   // ofRect(xPos, ofGetHeight()-20, 60, 20);
}

void Player::shoot() {
    
}


