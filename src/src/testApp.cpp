#include "testApp.h"

/*
 to-do:
 - phone: make app resettable
 - ensure all hitboxes work
 - add in explosions
 - add in music
 - fix ts
 - extra credit: add in some kind of recording
 */


//--------------------------------------------------------------
void testApp::setup() {
    
    titleFont.loadFont("arcade.ttf", 72);
    scoreFont.loadFont("arcade.ttf", 30);
    logo.loadImage("logo_sq.png");
    
    theme.loadSound("phantom.mp3");
    theme.play();
    
    
	ofSetLogLevel(OF_LOG_VERBOSE);
    
    // new kinect setup stuff
    isLive			= true;
	isTracking		= true;
	isTrackingHands	= true;
	isFiltering		= false;
	isRecording		= false;
	isCloud			= false;
	isCPBkgnd		= true;
	isMasking		= true;
    
	nearThreshold = 500;
	farThreshold  = 1000;
    
	filterFactor = 0.1f;
    
	setupRecording();
    
    // OSC port stuff--will need 2 for 2 players, duh
    receiverP1.setup(8014);
    receiverP2.setup(8015);
    
    //setting up the audio
    ofSoundStreamSetup(0, 2, this, 44100, 256, 4);
        
    isUsingKeyboard = false;
    reset();
    
    gameState = 0;
    
    startAngle = 0;
    
    for (int i = 0; i < 40; i++) {
        angles.push_back(startAngle);
        startAngle+=9;
    }
    
    
//    cout << boss.font.stringWidth("F") << endl;
//    cout << boss.font.stringHeight("F") << endl;
}

//--------------------------------------------------------------
void testApp::update() {
    bulletTimerP1++; // this can always go up without a problem
    bulletTimerP2++; // bullet timer
    enemyTimer++; // timer for enemies; only relevant if not using text file
    
	hardware.update(); // only works on mac at moment; just updating the kinect
    
    checkIfYelling(); // ALWAYS want to check if yelling. it's how we control anything, including starting, restarting, etc.
    compareYells();

    
    if (theme.getPosition() == 1) {
        theme.setPosition(0);
        theme.play();
    }
    
    // NEW KINECT STUFF; if there is a new frame and we are connected
    if (isLive) {
        // update all nodes
        recordContext.update();
        recordDepth.update();
        recordImage.update();
        
        // demo getting depth pixels directly from depth gen
        depthRangeMask.setFromPixels(recordDepth.getDepthPixels(nearThreshold, farThreshold),
                                     recordDepth.getWidth(), recordDepth.getHeight(), OF_IMAGE_GRAYSCALE);
        
        // update tracking/recording nodes
        if (isTracking) recordUser.update();
        if (isRecording) oniRecorder.update();
        
        // demo getting pixels from user gen
        if (isTracking && isMasking) {
            allUserMasks.setFromPixels(recordUser.getUserPixels(), recordUser.getWidth(), recordUser.getHeight(), OF_IMAGE_GRAYSCALE);
            user1Mask.setFromPixels(recordUser.getUserPixels(1), recordUser.getWidth(), recordUser.getHeight(), OF_IMAGE_GRAYSCALE);
            user2Mask.setFromPixels(recordUser.getUserPixels(2), recordUser.getWidth(), recordUser.getHeight(), OF_IMAGE_GRAYSCALE);
        }
    } else {
        // update all nodes
        playContext.update();
        playDepth.update();
        playImage.update();
        
        // demo getting depth pixels directly from depth gen
        depthRangeMask.setFromPixels(playDepth.getDepthPixels(nearThreshold, farThreshold),
                                     playDepth.getWidth(), playDepth.getHeight(), OF_IMAGE_GRAYSCALE);
        
        // update tracking/recording nodes
        if (isTracking) playUser.update();
        
        // demo getting pixels from user gen
        if (isTracking && isMasking) {
            allUserMasks.setFromPixels(playUser.getUserPixels(), playUser.getWidth(), playUser.getHeight(), OF_IMAGE_GRAYSCALE);
            user1Mask.setFromPixels(playUser.getUserPixels(1), playUser.getWidth(), playUser.getHeight(), OF_IMAGE_GRAYSCALE);
            user2Mask.setFromPixels(playUser.getUserPixels(2), playUser.getWidth(), playUser.getHeight(), OF_IMAGE_GRAYSCALE);
        }
    }
    
    
    
    
    // ----- titlescreen -----
    if (gameState == 0) {
        if (maxLevelP1 > micSensitivity || maxLevelP2 > micSensitivity) {
            gameState = 1;
            gameStartTime = ofGetElapsedTimeMillis();
        }
        addBullets();
        
    }
    
    //    use the below code if you just want enemies to pop out whenever (i.e. if you want to test general stuff, not levels)
//    if (enemyTimer > enemyTime) {
//        enemyTimer = 0;
//        Enemy e;
//        e.setup(ofRandom(ofGetWidth()), 0);
//        enemies.push_back(e);
//    }
    
    
    if (gameState == 1) {
        
        if (enemyTimeSpawn.size()>0){
            if (ofGetElapsedTimeMillis() - gameStartTime > enemyTimeSpawn[0]) {
                // spawn new enemy
                Enemy e;
                cout << "spawn" << endl;
                e.setup(enemyXPos[0], enemyYPos[0], enemyColors[0]);
                enemies.push_back(e);
                // delete from vector
                enemyXPos.erase(enemyXPos.begin());
                enemyYPos.erase(enemyYPos.begin());
                enemyTimeSpawn.erase(enemyTimeSpawn.begin());
                enemyColors.erase(enemyColors.begin());
            }
        }
    
    

                
        addBullets(); // adding bullets when yelling; moved to a function because unlikely to change much in this context
//        checkBullets(); // checking bullets; moved to a function because we won't really change it often
        
        // update our enemies
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].update();
        }
        
        player1.update();
        player2.update();
        
        // just in case we need to demo, i guess?
        if (isUsingKeyboard == false) {

        }
        
        // filling up the yells with the various volumes
        if (player1.hasStartedYelling == true) { //player is currently yelling.
            newYellP1.push_back(volumeP1);
        }
        
        if (player2.hasStartedYelling == true) {
            newYellP2.push_back(volumeP2);
        }
        
        // the number here will change (and maybe there's a cleverer way to do this), but in any case, here's when we can trigger the BAWSS
        if (ofGetElapsedTimeMillis() > 36000 + gameStartTime) {
            boss.bossOnScreen = true;
        }
        
        if (boss.bossOnScreen == true) {
            boss.update();
        }
    }
}


//--------------------------------------------------------------
bool testApp::compareYells() {
    float newYellMaxP1 = 0.0;
    float newYellMaxP2 = 0.0;
    float oldYellMax = 0.0;
    
    for (int i = 0; i < newYellP1.size(); i++) {
        if (newYellMaxP1 < newYellP1[i]) {
            newYellMaxP1 = newYellP1[i];
        }
    }
    
    for (int i = 0; i < newYellP2.size(); i++) {
        if (newYellMaxP2 < newYellP2[i]) {
            newYellMaxP2 = newYellP2[i];
        }
    }
    
    for (int i = 0; i < oldYell.size(); i++) {
        if (oldYellMax < oldYell[i]) {
            oldYellMax = oldYell[i];
        }
    }
    
//    cout << "old yell max " << oldYellMax << endl;
//    cout << "new yell max p1 " << newYellMaxP1 << endl;
//    cout << "new yell max p2 " << newYellMaxP2 << endl;
    
    if (newYellMaxP1 > oldYellMax) {
        // can escalate by moving the above code into here. that way you always have to top the loudest
        oldYell.clear();
        for (int i = 0; i < newYellP1.size(); i++) {
            oldYell.push_back(newYellP1[i]);
        }
        newYellP1.clear();
        player1.isWinning = true;
        player2.isWinning = false;
//        cout << "hooray P1" << endl;
        return true;
    } else if (newYellMaxP2 > oldYellMax) {
        oldYell.clear();
        for (int i = 0; i < newYellP2.size(); i++) {
            oldYell.push_back(newYellP2[i]);
        }
        newYellP2.clear();
        player2.isWinning = true;
        player1.isWinning = false;
//        cout << "hooray P2" << endl;
        return true;
    } else {
//        cout << "boo" << endl;
        return false;
        
    }
    
}

//--------------------------------------------------------------
void testApp::draw() {
    ofBackground(0);
    
    ofSetColor(255, 255, 255);
    if (isLive) {
        ofEnableAlphaBlending();
        ofPushMatrix();
        ofTranslate(0, 300);
        recordUser.draw();
        ofPopMatrix();
        ofSetColor(0, 0, 0, 150);
        ofRect(0, 0, 1024, 768);
    } 
    
    
    // here's where we actually set the positions, using the hip position. a bit wonky but we can tweak later
    
    if (recordUser.getNumberOfTrackedUsers() > 0) {
        cout << "tracking :" << recordUser.getNumberOfTrackedUsers() << endl;
        
        if (recordUser.getNumberOfTrackedUsers() == 1) {
            ofxTrackedUser* tracked1 = recordUser.getTrackedUser(1);
            if( recordUser.getXnUserGenerator().GetSkeletonCap().IsTracking(tracked1->id)){
                player1.xPos = tracked1->hip.position[1].X;
                printf("%f/",player1.xPos);
            }
        } else if (recordUser.getNumberOfTrackedUsers() == 2) {
            ofxTrackedUser* tracked1 = recordUser.getTrackedUser(1);
            if( recordUser.getXnUserGenerator().GetSkeletonCap().IsTracking(tracked1->id)){
                player1.xPos = tracked1->hip.position[1].X;
                printf("%f/",player1.xPos);
            }
            
            ofxTrackedUser* tracked2 = recordUser.getTrackedUser(2);
            if( recordUser.getXnUserGenerator().GetSkeletonCap().IsTracking(tracked2->id)){
                player2.xPos = tracked2->hip.position[1].X;
                printf("%f/",player2.xPos);
            }
   
        }
        
    }
    
    // ----- titlescreen -----
    if (gameState == 0) {
        
        float shake = 0;
        //shake = ofMap(maxLevelP1 + maxLevelP2, 0, .6, 0, 10);
        shake = ofRandom(1, 10);
        
        player1.draw();
        player2.draw();
        
        ofEnableAlphaBlending();
        ofSetColor(0, 0, 0, 150);
        ofRect(0, 0, ofGetWidth(), ofGetHeight());
        ofPushMatrix();
        
        // kulers
        ofColor c;
        c.setHsb(ofRandom(255), 255, 255, 255);
        ofSetColor(c);
        
        // drawin' der logo
        ofTranslate(ofRandom(shake), ofRandom(shake));
        logo.draw((ofGetWidth()/2) - (logo.width/2), 50);
        ofDisableAlphaBlending();
        ofPopMatrix();
        
        
        
// drawin' der lines
//        ofPushMatrix();
//        ofRotate(ofRandom(2));
//        // drawing lines?
//        for (int i = 0; i < angles.size(); i++) {
//            float nang = ofDegToRad(angles[i]);
//            ofSetLineWidth(4);
//            // this line will be relevant:   
//            //ofRect(ofGetWidth()/4.0, ofGetHeight(), 50, -ofMap(maxLevelP1, 0, micSensitivity, 0, ofGetHeight()-160));
//            ofLine(ofGetWidth()/2, ofGetHeight()/2, ofGetWidth()/2 + cos(nang)*500, ofGetHeight()/2+sin(nang)*500);
//        }
//        ofPopMatrix();

        
        if (blinkCounter%300 > 150) {
            ofSetColor(255);
            scoreFont.drawString("SCREAM TO START", ofGetWidth()/2 - 225, ofGetHeight() - 75);
        }
        

    
        
        blinkCounter++;
        
    }
    
    if (gameState == 1) {
        
//        grayImageOn;
        
        ofBackground(0);
        float shake = 0;
        if (maxLevelP1 > shakeSensitivity) {
            shake = ofMap(maxLevelP1, shakeSensitivity, .5, 0, maxShake); 
        } else if (maxLevelP2 > shakeSensitivity) {
            shake = ofMap(maxLevelP2, shakeSensitivity, .5, 0, maxShake);
        }
        
        ofPushMatrix();
        ofTranslate(ofRandom(shake), ofRandom(shake));
        ofSetColor(255, 255, 255);
        
        
//        if (grayImageOn) {
//            grayDiff.draw(0, 0, grayDiff.width*gameScale, grayDiff.height*gameScale);
//        }
        
        if (enemyTimer > enemyTime) {
            enemyTimer = 0;
            ofVec2f s;
            s.x = ofRandom(ofGetWidth());
            s.y = 0;
            regularStars.push_back(s);
        }
        
        for (int i = 0; i < regularStars.size(); i++) {
            regularStars[i].y+=2;
            ofSetColor(255);
            ofRect(regularStars[i].x, regularStars[i].y, 2, 2);
        }
        
        //game stuff! 
        checkBullets(); // checking bullets; moved to a function because we won't really change it often
        
//        if (player1.isWinning) {
//            ofDrawBitmapString("Player 1 is louder!", ofGetWidth()/(ofRandom(1.98,2)), 100);
//           // player1.numKilled+=1;
//        }
//        
//        if (player2.isWinning) {
//            ofDrawBitmapString("Player 2 is louder!", ofGetWidth()/(ofRandom(1.98,2)), 100);
//           // player2.numKilled+=2;
//        }
        
        player1.draw();
        player2.draw();
        
        for (int i = 0; i < bulletsP1.size(); i++) {
            bulletsP1[i].draw();
        }
        
        for (int i = 0; i < bulletsP2.size(); i++) {
            bulletsP2[i].draw();
        }
        
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].draw();
        }
        
        if (boss.bossOnScreen == true) {
            boss.draw();
        }
        
        // let's print the ratio of enemies shot to total enemies on screen.
        ofSetColor(255);
        scoreFont.drawString("Player 1: " + ofToString(player1.numKilled), 70, 50);
        scoreFont.drawString("Player 2: " + ofToString(player2.numKilled), ofGetWidth()/2+70, 50);
        
        ofPopMatrix();
        
        if (boss.health < 0 || boss.yPos >= ofGetHeight()) {
            gameState = 3;
        }
    }
    
    if (gameState == 3) {
        if (boss.health <= 0) {
            titleFont.drawString("PWNED!!!1", ofGetWidth()/3, ofGetHeight()/2);
        } else {
            titleFont.drawString("BOSS\nESCAPED", ofGetWidth()/3, ofGetHeight()/2);
        }
        
        if (player1.numKilled > player2.numKilled) {
            scoreFont.drawString("PLAYER 1 WINS!!1~", ofGetWidth()/3, 500);
        } else {
            scoreFont.drawString("PLAYER 2 WINS!!1~", ofGetWidth()/3, 600);   
        }
    }
}

//--------------------------------------------------------------

void testApp::checkIfYelling() {
    
    if (maxLevelP1 > micSensitivity && player1.hasStartedYelling == false) {
        player1.hasStartedYelling = true;
        cout << "start yelling" << endl;
    } 
    
    // is P2 yelling?
    if (maxLevelP2 > micSensitivity && player2.hasStartedYelling == false) {
        player2.hasStartedYelling = true;
        cout << "start yelling" << endl;
    }
    
    // if they have stopped yelling (effectively--they're not hitting the threshold), we should compare their yells
    // p1
    if (player1.hasStartedYelling == true && maxLevelP1 <= micSensitivity){
        player1.hasStartedYelling = false;
        cout << "stopyelling" << endl;
        //do whatever should happen when they stop yelling
        compareYells();
    }
    // p2
    if (player2.hasStartedYelling == true && maxLevelP2 <= micSensitivity){
        player2.hasStartedYelling = false;
        cout << "stopyelling" << endl;
        //do whatever should happen when they stop yelling
        compareYells();
    }
    
}


//--------------------------------------------------------------


void testApp::audioReceived (float * input, int bufferSize, int nChannels){   
    maxLevel = 0.0f; //reset max to minimum each snd buffer  
    for (int i = 0; i < bufferSize; i++){  
        float abs1 = ABS(input[i*2]);   
        if (maxLevel < abs1) maxLevel = abs1; // grab highest value  
    }  
}

//--------------------------------------------------------------

void testApp::checkBullets() {
    // checking P1 bullets against enemies
    for (int i = 0; i < enemies.size(); i++) {
        for (int k = 0; k < bulletsP1.size(); k++) {
            if (ofDist(enemies[i].xPos, enemies[i].yPos, bulletsP1[k].xPos, bulletsP1[k].yPos) < 30) {
                enemies.erase(enemies.begin()+i);
                bulletsP1.erase(bulletsP1.begin()+i);
                
                if (abs(bulletsP1[k].color.getHue() - enemies[i].eColor.getHue()) < 30) {
                    ofSetColor(255);
                    titleFont.drawString("COLOR BONUS!!!1", 70, ofGetHeight()/2);
                    cout << "bonussssss" << endl;
                    player1.numKilled+=150;
                } else {
                    player1.numKilled+=50;
                }
                
            }
        }
    }
    
    // checking P2 bullets against enemies
    for (int i = 0; i < enemies.size(); i++) {
        for (int k = 0; k < bulletsP2.size(); k++) {
            if (ofDist(enemies[i].xPos, enemies[i].yPos, bulletsP2[k].xPos, bulletsP2[k].yPos) < 30) {
                enemies.erase(enemies.begin()+i);
                bulletsP2.erase(bulletsP2.begin()+i);
                player2.numKilled++;
                
                if (abs(bulletsP2[k].color.getHue() - enemies[i].eColor.getHue()) < 30) {
                    ofSetColor(255);
                    titleFont.drawString("COLOR BONUS!!!1", ofGetHeight()+70, ofGetHeight()/2);
                    cout << "bonussssss" << endl;
                    player2.numKilled+=5;
                } else {
                    player2.numKilled++;
                }
            }
        }
    }
    
    // updating bullets and deleting them if they're offscreen
    // p1
    for (int i = 0; i < bulletsP1.size(); i++) {
        bulletsP1[i].update();
        if (bulletsP1[i].yPos < 0) {
            bulletsP1.erase(bulletsP1.begin()+i); // iterator helps you quickly access memory locations; this points to first slot, and then you just hop over to yours
        }
    }
    
    // updating bullets and deleting them if they're offscreen
    for (int i = 0; i < bulletsP2.size(); i++) {
        bulletsP2[i].update();
        if (bulletsP2[i].yPos < 0) {
            bulletsP2.erase(bulletsP2.begin()+i); // iterator helps you quickly access memory locations; this points to first slot, and then you just hop over to yours
        }
    }
    
    if (boss.bossOnScreen == true && boss.shieldsUp == false) {
        for (int i = 0; i < bulletsP1.size(); i++) {
            if (ofDist(bulletsP1[i].xPos, bulletsP1[i].yPos, boss.xPos + 170, boss.yPos - 113) < 170) {
                boss.health-=2;
                player1.numKilled+=200;
                bulletsP1.erase(bulletsP1.begin()+i);
            }
        }
        
        for (int i = 0; i < bulletsP2.size(); i++) {
            if (ofDist(bulletsP2[i].xPos, bulletsP2[i].yPos, boss.xPos, boss.yPos) < 30) {
                boss.health--;
                player2.numKilled+=200;
                bulletsP2.erase(bulletsP2.begin()+i);
            }
        }
    }
}

//--------------------------------------------------------------

void testApp::addBullets() {
    // this should be where we should be taking our OSC messages and passing them into b.setup() to change stuff.
    
    // so let's get our osc messages first.
    while (receiverP1.hasWaitingMessages()) {
        cout<<"got from 1"<<endl;
        ofxOscMessage mP1;
        receiverP1.getNextMessage(&mP1);
        // and now we get the volume from MAH APP
        if (mP1.getAddress() == "/volume/max") {
            volumeP1=mP1.getArgAsFloat(0);
            cout << volumeP1 << endl;
            maxLevelsP1.push_back(volumeP1);
            if (maxLevelsP1.size()>numLevelsToStore) {
                maxLevelsP1.erase(maxLevelsP1.begin());
            }
        } else if (mP1.getAddress() == "/fft/levels") {
            p1MaxLocForFFT = (int)mP1.getArgAsFloat(0); // herp derp this should really be an int, but w/e
        } else {
            // for unrecognized messages. likely not needed but probably useful for debugging
        }
    }
    
    while (receiverP2.hasWaitingMessages()) {
        cout<<"got from 2"<<endl;
        ofxOscMessage mP2;
        receiverP2.getNextMessage(&mP2);
        // and now we get the volume from the app
        if (mP2.getAddress() == "/volume/max") {
            volumeP2=mP2.getArgAsFloat(0);
            maxLevelsP2.push_back(volumeP2);
            if (maxLevelsP2.size()>numLevelsToStore) {
                maxLevelsP2.erase(maxLevelsP2.begin());
            }
        } else if (mP2.getAddress() == "/fft/levels") {
            p2MaxLocForFFT = (int)mP2.getArgAsFloat(0); // herp derp this should really be an int, but w/e
        } else {
            // for unrecognized messages. likely not needed but probably useful for debugging
        }
    }
    
    //set max level as the average of our recently recorded levels
    //p1
    maxLevelP1=0.0;
    for (int i=0; i<maxLevelsP1.size(); i++){
        maxLevelP1+= maxLevelsP1[i];
    }
    maxLevelP1/= (float)maxLevelsP1.size();
    
    //p2
    maxLevelP2=0.0;
    for (int i=0; i<maxLevelsP2.size(); i++){
        maxLevelP2+= maxLevelsP2[i];
    }
    maxLevelP2/= (float)maxLevelsP2.size();
    
    int diffBetweenMaxLoc = abs(p1MaxLocForFFT - p2MaxLocForFFT);
    // we will probably want to pass that to the bullet and map it onto velocity, size, wiggliness, etc. etc. but for now let's just see if they match at all. remember, we'll probably want to have a vector so we can see if they're sustaining the same frequency over time.
    
    if (gameState == 1) {
        // and now we'll pass the f out of this s to the bullet setup function.
        if (player1.hasStartedYelling) {
            if (bulletTimerP1 > bulletTimeP1) {
                bulletTimerP1 = 0; //resetting the timer
                Bullet b;
                b.setup(ofMap(maxLevelP1, 0, .9, 5, 15), p1MaxLocForFFT, diffBetweenMaxLoc, player1.xPos);
                bulletsP1.push_back(b);
            }
        }
        
        if (player2.hasStartedYelling) {
            if (bulletTimerP2 > bulletTimeP2) {
                bulletTimerP2 = 0; //resetting the timer
                Bullet b;
                b.setup(ofMap(maxLevelP2, 0, .9, 5, 15), p2MaxLocForFFT, diffBetweenMaxLoc, player2.xPos);
                bulletsP2.push_back(b);
            }
        }
    }
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
	float smooth;
    
	switch (key) {
#ifdef TARGET_OSX // only working on Mac at the moment
		case 357: // up key
			hardware.setTiltAngle(hardware.tilt_angle++);
			break;
		case 359: // down key
			hardware.setTiltAngle(hardware.tilt_angle--);
			break;
#endif
        case 'y':
            //put anything you want here
            break;
		case 's':
		case 'S':
			if (isRecording) {
				oniRecorder.stopRecord();
				isRecording = false;
				break;
			} else {
				oniRecorder.startRecord(generateFileName());
				isRecording = true;
				break;
			}
			break;
		case 'p':
		case 'P':
			if (oniRecorder.getCurrentFileName() != "" && !isRecording && isLive) {
				setupPlayback(oniRecorder.getCurrentFileName());
				isLive = false;
			} else {
				isLive = true;
			}
			break;
		case 't':
		case 'T':
			isTracking = !isTracking;
			break;
		case 'h':
		case 'H':
			isTrackingHands = !isTrackingHands;
			if(isLive) recordHandTracker.toggleTrackHands();
			if(!isLive) playHandTracker.toggleTrackHands();
			break;
		case 'f':
		case 'F':
			isFiltering = !isFiltering;
			recordHandTracker.isFiltering = isFiltering;
			playHandTracker.isFiltering = isFiltering;
			break;
		case 'm':
		case 'M':
			isMasking = !isMasking;
			recordUser.setUseMaskPixels(isMasking);
			playUser.setUseMaskPixels(isMasking);
			break;
		case 'c':
		case 'C':
			isCloud = !isCloud;
			recordUser.setUseCloudPoints(isCloud);
			playUser.setUseCloudPoints(isCloud);
			break;
		case 'b':
		case 'B':
			isCPBkgnd = !isCPBkgnd;
			break;
		case '9':
		case '(':
			smooth = recordUser.getSmoothing();
			if (smooth - 0.1f > 0.0f) {
				recordUser.setSmoothing(smooth - 0.1f);
				playUser.setSmoothing(smooth - 0.1f);
			}
			break;
		case '0':
		case ')':
			smooth = recordUser.getSmoothing();
			if (smooth + 0.1f <= 1.0f) {
				recordUser.setSmoothing(smooth + 0.1f);
				playUser.setSmoothing(smooth + 0.1f);
			}
			break;
		case '[':
            //case '{':
			if (filterFactor - 0.1f > 0.0f) {
				filterFactor = filterFactor - 0.1f;
				recordHandTracker.setFilterFactors(filterFactor);
				if (oniRecorder.getCurrentFileName() != "") playHandTracker.setFilterFactors(filterFactor);
			}
			break;
		case ']':
            //case '}':
			if (filterFactor + 0.1f <= 1.0f) {
				filterFactor = filterFactor + 0.1f;
				recordHandTracker.setFilterFactors(filterFactor);
				if (oniRecorder.getCurrentFileName() != "") playHandTracker.setFilterFactors(filterFactor);
			}
			break;
		case ';':
		case ':':
			smooth = recordHandTracker.getSmoothing();
			if (smooth - 0.1f > 0.0f) {
				recordHandTracker.setSmoothing(smooth -  0.1f);
				playHandTracker.setSmoothing(smooth -  0.1f);
			}
			break;
		case '\'':
		case '\"':
			smooth = recordHandTracker.getSmoothing();
			if (smooth + 0.1f <= 1.0f) {
				recordHandTracker.setSmoothing(smooth +  0.1f);
				playHandTracker.setSmoothing(smooth +  0.1f);
			}
			break;
		case '>':
		case '.':
			farThreshold += 50;
			if (farThreshold > recordDepth.getMaxDepth()) farThreshold = recordDepth.getMaxDepth();
			break;
		case '<':
		case ',':
			farThreshold -= 50;
			if (farThreshold < 0) farThreshold = 0;
			break;
            
		case '+':
		case '=':
			nearThreshold += 50;
			if (nearThreshold > recordDepth.getMaxDepth()) nearThreshold = recordDepth.getMaxDepth();
			break;
            
		case '-':
		case '_':
			nearThreshold -= 50;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
		case 'r':
			recordContext.toggleRegisterViewport();
			break;
		default:
			break;
	}
}


//void testApp::keyPressed (int key) {
//    switch (key) {
//        case ' ':
//            bLearnBackground = true;
//            //bThreshWithOpenCV = !bThreshWithOpenCV;
//            break;
//            
////        case 'w':
////            kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
////            break;
//            
////        case 'o':
////            kinect.setCameraTiltAngle(angle); // go back to prev tilt
////            kinect.open();
////            break;
//            
//        case 'c':
//            kinect.setCameraTiltAngle(0); // zero the tilt
//            kinect.close();
//            break;
//            
//        case OF_KEY_UP:
//            angle++;
//            if(angle>30) angle=30;
//            kinect.setCameraTiltAngle(angle);
//            break;
//            
//        case OF_KEY_DOWN:
//            angle--;
//            if(angle<-30) angle=-30;
//            kinect.setCameraTiltAngle(angle);
//            break;
//            
//        case '=':
//            threshold++;
//            break;
//            
//        case '-':
//            threshold--;
//            break;
//            
//        case 'a':
//            micSensitivity+=.05;
//            break;
//            
//        case 'z':
//            micSensitivity-=.05;
//            break;
//            
//        case 'e':
//            oldYell.clear();
//            player1.isWinning = false;
//            player2.isWinning = false;
//            break;
//            
//        case 'd':
//            grayImageOn = !grayImageOn;
//            break;
//            
//        case 'r':
//            reset();
//            break;
//            
//        case 'k':
//            isUsingKeyboard = !isUsingKeyboard;
//            break;
//            
//    }
//    
//    if (key == 'q') {
//        player1.isMovingLeft = true;
//        player1.isMovingRight = false;
//    }
//    
//    if (key == 'w') {
//        player1.isMovingRight = true;
//        player1.isMovingLeft = false;
//    }
//    
//    if (key == 'o') {
//        player2.isMovingLeft = true;
//        player2.isMovingRight = false;
//    }
//    
//    if (key == 'p') {
//        player2.isMovingRight = true;
//        player2.isMovingLeft = false;
//    }
//    
//}
//--------------------------------------------------------------
void testApp::loadFromText() {
    // clear vectors to make it not fuck up in between games/quit games
    enemies.clear();
    enemyXPos.clear();
    enemyYPos.clear();
    enemyTimeSpawn.clear();
    enemyColors.clear();
    
    
    ifstream fin; // input file stream; file in
    vector<string> enemyLines; // vector holding information
    
    fin.open(ofToDataPath("enemies.txt").c_str()); // of looks at bin by default; c-style string (whole thing is like chars)
    
    while (fin!=NULL) { // as long as it's a) not empty and b) at the end of the file
        
        string textFromLine; // temporary string to hold...
        getline(fin, textFromLine); // get the line from fin and store it in temp variable textfromline
        enemyLines.push_back(textFromLine);
    }
    
    for (int i = 0; i < enemyLines.size(); i+=5) {
        int newX = atoi(enemyLines[i].c_str());
        enemyXPos.push_back(newX);
        
        int newY = atoi(enemyLines[i+1].c_str());
        enemyYPos.push_back(newY);
        
        int newTime = atoi(enemyLines[i+2].c_str());
        enemyTimeSpawn.push_back(newTime);
        cout<<"this foe time: "<<newTime<<endl;
        
        string newColor = enemyLines[i+3];
        enemyColors.push_back(newColor);
        cout << newColor << endl;
        
    }
    
    cout<<"done loading"<<endl;
    
}
//--------------------------------------------------------------
void testApp::reset() {
    //game stuff!----------------------
    gameState = 0;
    
    bulletTimerP1 = 0;
    bulletTimeP1 = 2; //frames
    
    bulletTimerP2 = 0;
    bulletTimeP2 = 2; //frames
    
    enemiesShot = 0;
    
//    gameScale = ofGetScreenWidth()/kinect.width;
    
    numLevelsToStore = 50;
    
    //    micSensitivity = .2;
    micSensitivity = .3;
    
    inputBufferCopy = new float[512*2];
    
    oldYell.push_back(0.0f);
    
    shakeSensitivity = micSensitivity*1.2;
    maxShake = 5;
    
    grayImageOn = true;
    
    // calling text file for waves of enemies
    loadFromText();
    
    enemyTimer = 0;
    //    enemyTime = 30;
    enemyTime = 15;
    
    ofEnableAlphaBlending();
    
    for (int i = 0; i < 30; i++) {
        ofVec2f s;
        s.x = ofRandom(ofGetWidth());
        s.y = ofRandom(ofGetHeight());
        regularStars.push_back(s);
    }
    
    boss.setup();
    gameStartTime = ofGetElapsedTimeMillis();
    
    player1.numKilled = 0;
    player2.numKilled = 0;

    
}
//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{}


// PUTTING THE FOLLOWING AT THE BOTTOM B/C UNLIKELY TO BE CHANGED
// --------------------------------------------------------------
void testApp::setupRecording(string _filename) {
    
#if defined (TARGET_OSX) //|| defined(TARGET_LINUX) // only working on Mac/Linux at the moment (but on Linux you need to run as sudo...)
	hardware.setup();				// libusb direct control of motor, LED and accelerometers
	hardware.setLedOption(LED_OFF); // turn off the led just for yacks (or for live installation/performances ;-)
#endif
    
	recordContext.setup();	// all nodes created by code -> NOT using the xml config file at all
	//recordContext.setupUsingXMLFile();
	recordDepth.setup(&recordContext);
	recordImage.setup(&recordContext);
    
	recordUser.setup(&recordContext);
	recordUser.setSmoothing(filterFactor);				// built in openni skeleton smoothing...
	recordUser.setUseMaskPixels(isMasking);
	recordUser.setUseCloudPoints(isCloud);
	recordUser.setMaxNumberOfUsers(2);					// use this to set dynamic max number of users (NB: that a hard upper limit is defined by MAX_NUMBER_USERS in ofxUserGenerator)
    
	recordHandTracker.setup(&recordContext, 4);
	recordHandTracker.setSmoothing(filterFactor);		// built in openni hand track smoothing...
	recordHandTracker.setFilterFactors(filterFactor);	// custom smoothing/filtering (can also set per hand with setFilterFactor)...set them all to 0.1f to begin with
    
	recordContext.toggleRegisterViewport();
	recordContext.toggleMirror();
    
	oniRecorder.setup(&recordContext, ONI_STREAMING);
	//oniRecorder.setup(&recordContext, ONI_CYCLIC, 60);
	//read the warning in ofxOpenNIRecorder about memory usage with ONI_CYCLIC recording!!!
    
}
// --------------------------------------------------------------
void testApp::setupPlayback(string _filename) {
    
	playContext.shutdown();
	playContext.setupUsingRecording(ofToDataPath(_filename));
	playDepth.setup(&playContext);
	playImage.setup(&playContext);
    
	playUser.setup(&playContext);
	playUser.setSmoothing(filterFactor);				// built in openni skeleton smoothing...
	playUser.setUseMaskPixels(isMasking);
	playUser.setUseCloudPoints(isCloud);
    
	playHandTracker.setup(&playContext, 4);
	playHandTracker.setSmoothing(filterFactor);			// built in openni hand track smoothing...
	playHandTracker.setFilterFactors(filterFactor);		// custom smoothing/filtering (can also set per hand with setFilterFactor)...set them all to 0.1f to begin with
    
	playContext.toggleRegisterViewport();
	playContext.toggleMirror();
    
}
// --------------------------------------------------------------
void testApp:: drawMasks() {
	glPushMatrix();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	allUserMasks.draw(640, 0, 640, 480);
	glDisable(GL_BLEND);
    glPopMatrix();
	user1Mask.draw(320, 480, 320, 240);
	user2Mask.draw(640, 480, 320, 240);
	
}
// --------------------------------------------------------------
void testApp::drawPointCloud(ofxUserGenerator * user_generator, int userID) {
    
	glPushMatrix();
    
	int w = user_generator->getWidth();
	int h = user_generator->getHeight();
    
	glTranslatef(w, h/2, -500);
	ofRotateY(pointCloudRotationY);
    
	glBegin(GL_POINTS);
    
	int step = 1;
    
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			ofPoint pos = user_generator->getWorldCoordinateAt(x, y, userID);
			if (pos.z == 0 && isCPBkgnd) continue;	// gets rid of background -> still a bit weird if userID > 0...
			ofColor color = user_generator->getWorldColorAt(x,y, userID);
			glColor4ub((unsigned char)color.r, (unsigned char)color.g, (unsigned char)color.b, (unsigned char)color.a);
			glVertex3f(pos.x, pos.y, pos.z);
		}
	}
    
	glEnd();
    
	glColor3f(1.0f, 1.0f, 1.0f);
    
	glPopMatrix();
}
//--------------------------------------------------------------
string testApp::generateFileName() {
    
	string _root = "kinectRecord";
    
	string _timestamp = ofToString(ofGetDay()) +
	ofToString(ofGetMonth()) +
	ofToString(ofGetYear()) +
	ofToString(ofGetHours()) +
	ofToString(ofGetMinutes()) +
	ofToString(ofGetSeconds());
    
	string _filename = (_root + _timestamp + ".oni");
    
	return _filename;
    
}
