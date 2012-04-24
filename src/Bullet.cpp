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

void Bullet::setup(float r, int f, int d, float x) {
    
    // now, the question is, do we also want to pass the original value so each players bullets have a hint of the original frequency? you know? like, so the player's bullets also at least partially reflect their sounds. the answer is probably.
    
    xPos = x;
    yPos = ofGetHeight();
    
    yVelocity = 25;
    radius = r;

    difference = d;
    
    playerF = ofMap(f, 0, 50, 0, 255);

}

void Bullet::update() {
    yPos -= yVelocity;
}

void Bullet::draw() {
    
    // let's change the bullet color based on the frequency you're making--gonna wanna change from d to your f
    ofFill();
    ofSetColor(playerF, 150, 150);
    
    // let's change the bullet shape based on difference in frequency--more pointy the more different they are
    // is round the best thing to do here? is it different than just casting it as an int?
    cout << round(ofMap(difference, 0, 50, 0, 3)) << endl;
    
    // it would be awesome if we could do this as a shape with x# of vertices. if there's time! could also switch between curve and not curve
    if (ofMap(difference, 0, 50, 0, 3) <= .75) {
        ofEllipse(xPos, yPos, radius, radius);
    } else if (ofMap(difference, 0, 50, 0, 3) > .75 && ofMap(difference, 0, 50, 0, 3) <= 1.5) {
        ofRect(xPos, yPos, radius, radius);
    } else if (ofMap(difference, 0, 50, 0, 3) > 1.5 && ofMap(difference, 0, 50, 0, 3)) {
        ofTriangle(xPos - radius, yPos, xPos + radius, yPos, xPos, yPos-radius);
    }
}
