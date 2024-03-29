#include "testApp.h"

/*
 to-do:
 - fix hitboxes (boss mainly)
 - persistent high-score
 
 - extra credit: add in explosions
 - extra credit: add in some kind of recording
 */


//--------------------------------------------------------------
void testApp::setup() {

// game fonts and music
    titleFont.loadFont("arcade.ttf", 72);
    scoreFont.loadFont("arcade.ttf", 30);
    calibrateFont.loadFont("arcade.ttf", 20);
    logo.loadImage("logo_sq.png");
    theme.loadSound("phantom.mp3");
    //theme.play();
    
    
	ofSetLogLevel(OF_LOG_VERBOSE);
    
// new kinect setup stuff
    isLive			= true;
	isTracking		= true;
	isTrackingHands	= false;
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
    receiverP1.setup(8000);
    receiverP2.setup(8001);
    
// setting up the audio
    ofSoundStreamSetup(0, 2, this, 44100, 256, 4);


// game variables
    reset(); // resetting the game
    gameState = 3; // starting at title screen

// defining game parameters under the hood
    ofSetFrameRate(300);
    ofSetVerticalSync(false);
    
}

//--------------------------------------------------------------
void testApp::update() {
    
// ---- ALWAYS -----
// game timers
    bulletTimerP1++; // this can always go up without a problem
    bulletTimerP2++; // bullet timer
    
// game functions we want running at all times
    checkIfYelling(); // ALWAYS want to check if yelling. it's how we control anything, including starting, restarting, etc.
    
// making sure the theme music is always playing (since loop never really seems to work)
    if (theme.getPosition() == 1) {
        theme.setPosition(0);
        theme.play();
    }

// NEW KINECT STUFF
    hardware.update(); // only works on mac at moment; just updating the kinect
    
    if (isLive) {
        if (blinkCounter%5==0) {
        // update all nodes
        recordContext.update();
        recordDepth.update();
        recordImage.update();
        
        // update tracking/recording nodes
            if (isTracking) { 
                recordUser.update();
            }
        }
    } 
    
    // here's where we actually set the positions, using the hip position
    if (recordUser.getNumberOfTrackedUsers() > 0) { // if anyone is tracked
        cout << "tracking :" << recordUser.getNumberOfTrackedUsers() << endl; // print that ish out
        
        if (recordUser.getNumberOfTrackedUsers() == 2) { // if there are two users
            ofxTrackedUser* trackedA = recordUser.getTrackedUser(1); // one of the tracked users
            ofxTrackedUser* trackedB = recordUser.getTrackedUser(2); // and another
            
            if (recordUser.getXnUserGenerator().GetSkeletonCap().IsTracking(trackedA->id) && recordUser.getXnUserGenerator().GetSkeletonCap().IsTracking(trackedB->id)) { // honestly no idea what this does
                if (trackedA->hip.position[1].X < trackedB->hip.position[1].X) { // if A is less than B
                    player1.xPos = ofMap(trackedA->hip.position[1].X, 0, 640, 0, 1024); // player 1 is A
                    player1.isActive = true;
                    player2.xPos = ofMap(trackedB->hip.position[1].X, 0, 640, 0, 1024); // player 2 is B
                    player2.isActive = true;
                } else { // otherwise
                    player1.xPos = ofMap(trackedB->hip.position[1].X, 0, 640, 0, 1024); // player 1 is B
                    player1.isActive = true;
                    player2.xPos = ofMap(trackedA->hip.position[1].X, 0, 640, 0, 1024); // player 2 is A
                    player2.isActive = true;
                }
            }
        } else if (recordUser.getNumberOfTrackedUsers() == 1) { // but if there's only one user
            ofxTrackedUser* trackedA = recordUser.getTrackedUser(1); // figure out where they are
            if(recordUser.getXnUserGenerator().GetSkeletonCap().IsTracking(trackedA->id)){ // do magic
                if (ofMap(trackedA->hip.position[1].X, 0, 640, 0, 1024) < ofGetWidth()/2) { // if they're to the left of the midpoint
                    player1.xPos = ofMap(trackedA->hip.position[1].X, 0, 640, 0, 1024); // they should be player 1
                    player1.isActive = true;
                    player2.isActive = false;
                } else { // otherwise
                    player2.xPos = ofMap(trackedA->hip.position[1].X, 0, 640, 0, 1024); // they should be player 2
                    player2.isActive = true;
                    player1.isActive = false;
                }
            }
        }
        
    } else {
        player1.isActive = false;
        player2.isActive = false;
    }
    
    
// ----- TITLESCREEN -----
    if (gameState == 0) {
        if ((maxLevelP1 > micSensitivity && maxLevelP2 > micSensitivity) && (player1.isActive && player2.isActive) ) {
            gameState = 1;
            gameStartTime = ofGetElapsedTimeMillis();
        }
        addBullets();
        
    }
    
    
// ----- GAME ON -----
    if (gameState == 1) {
        
        // spawning our enemies from the text file
        if (enemyTimeSpawn.size()>0){
            if (ofGetElapsedTimeMillis() - gameStartTime > enemyTimeSpawn[0]) {
                // spawn new enemy
                Enemy e;
                e.setup(enemyXPos[0], enemyYPos[0], enemyColors[0]);
                enemies.push_back(e);
                // delete from vector
                enemyXPos.erase(enemyXPos.begin());
                enemyYPos.erase(enemyYPos.begin());
                enemyTimeSpawn.erase(enemyTimeSpawn.begin());
                enemyColors.erase(enemyColors.begin());
            }
        }
    
        // adding and checking bullets
        addBullets();
        checkBullets();
        
        // update our enemies
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].update();
        }
        
        // update our player positions
        player1.update();
        player2.update();
        
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
void testApp::draw() {

// ----- ALWAYS -----
    
    ofBackground(0);
    
// ----- TITLESCREEN -----
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
        
        ofSetColor(255);
        
        // when they're not ready
        if (blinkCounter%300 > 150) {
            if (!player1.isActive) {
                calibrateFont.drawString("CALIBRATE PLAYER 1", 70, ofGetHeight()-50);
            }
            if (!player2.isActive) {
                calibrateFont.drawString("CALIBRATE PLAYER 2", ofGetWidth()/2 + 70, ofGetHeight()-50);   
            }
        }
        
        
        
        // when they're ready
        if (recordUser.getNumberOfTrackedUsers() > 0) {
            if (player1.isActive) {
                calibrateFont.drawString("PLAYER 1 READY!", 90, ofGetHeight()-50);
            }
            
            if (player2.isActive) {
                calibrateFont.drawString("PLAYER 2 READY!", ofGetWidth()/2+90, ofGetHeight()-50);
            }
        }
        
        
        
        if (player1.isActive && player2.isActive) {
            if (blinkCounter%300 > 150) {
                ofSetColor(255);
                scoreFont.drawString("SCREAM TO START!", ofGetWidth()/2 - 225, ofGetHeight() - 150);
            }
        }
        

    
        // blinking whatever text needs to be blinked
        blinkCounter++;
        
    }
    
// ----- GAME ON -----
    if (gameState == 1) {
        // clear any old color/tints
        ofSetColor(255, 255, 255);
        
        // shaking the screen based on how loud the players are
        float shake = 0;
        if (maxLevelP1 > shakeSensitivity) {
            shake = ofMap(maxLevelP1, shakeSensitivity, .5, 0, maxShake); 
        } else if (maxLevelP2 > shakeSensitivity) {
            shake = ofMap(maxLevelP2, shakeSensitivity, .5, 0, maxShake);
        }
        ofPushMatrix();
        ofTranslate(ofRandom(shake), ofRandom(shake));
        
        // drawing our players
        player1.draw();
        player2.draw();
        
        // drawing our bullets
        for (int i = 0; i < bulletsP1.size(); i++) {
            bulletsP1[i].draw();
        }
        
        for (int i = 0; i < bulletsP2.size(); i++) {
            bulletsP2[i].draw();
        }
        
        // drawing our enemies
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].draw();
        }
        
        // drawing our boss
        if (boss.bossOnScreen == true) {
            boss.draw();
        }
        
        // let's print the ratio of enemies shot to total enemies on screen.
        ofSetColor(255);
        scoreFont.drawString("Player 1: " + ofToString(player1.numKilled), 70, 50);
        scoreFont.drawString("Player 2: " + ofToString(player2.numKilled), ofGetWidth()/2+70, 50);
        
        // ending our shake-translate
        ofPopMatrix();
        
        // switching based on whether you kill the boss or he gets away
        if (boss.health < 0 || boss.yPos >= ofGetHeight()) {
            gameState = 3;
        }
    }

// ----- END SCREEN -----
    if (gameState == 3) {
        
        // blinking whatever text needs to be blinked
        blinkCounter++; // we want the winner to be blinked
        
        float shake = 0; // let's shake in this part of the game as well
        shake = ofRandom(1, 5);
        
        ofPushMatrix();
        ofTranslate(ofRandom(shake), ofRandom(shake));
        // let's print the ratio of enemies shot to total enemies on screen.
        ofSetColor(255);
        scoreFont.drawString("Player 1: " + ofToString(player1.numKilled), 70, 50);
        scoreFont.drawString("Player 2: " + ofToString(player2.numKilled), ofGetWidth()/2+70, 50);
        
        // didja kill him?
        if (boss.health <= 0) {
            titleFont.drawString("BOSS: PWNED!!!1", ofGetWidth()/4, ofGetHeight()/2);
        } else {
            titleFont.drawString("BOSS: ESCAPED :(", ofGetWidth()/4, ofGetHeight()/2);
        }
        
        // who won?
        if (player1.numKilled > player2.numKilled) {
            scoreFont.drawString("WINNER: PLAYER 1 !!1~", ofGetWidth()/4, 600);
        } else if (player2.numKilled > player1.numKilled) {
            scoreFont.drawString("WINNER: PLAYER 2!!1~", ofGetWidth()/4, 600);   
        } else if (player2.numKilled == player1.numKilled) {
            scoreFont.drawString("WINNER: TIE!11!~!", ofGetWidth()/4, 600);
        }
        
        if (blinkCounter%300 > 100) {
            scoreFont.drawString("THANKS FOR PLAYING!", ofGetWidth()/4, 700);
        }
        ofPopMatrix();
    }
}

//--------------------------------------------------------------

void testApp::checkIfYelling() {
// are they yelling?    
    // is P1 yelling?
    if (maxLevelP1 > micSensitivity && player1.hasStartedYelling == false) {
        player1.hasStartedYelling = true;
    } 
    
    // is P2 yelling?
    if (maxLevelP2 > micSensitivity && player2.hasStartedYelling == false) {
        player2.hasStartedYelling = true;
    }
    
// if they have stopped yelling (effectively--they're not hitting the threshold), we should compare their yells
    // p1
    if (player1.hasStartedYelling == true && maxLevelP1 <= micSensitivity){
        player1.hasStartedYelling = false;
    }
    // p2
    if (player2.hasStartedYelling == true && maxLevelP2 <= micSensitivity){
        player2.hasStartedYelling = false;
    }
    
}

//--------------------------------------------------------------

void testApp::checkBullets() {
// seeing if any bullets hit any enemies
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
    
    // p2
    for (int i = 0; i < bulletsP2.size(); i++) {
        bulletsP2[i].update();
        if (bulletsP2[i].yPos < 0) {
            bulletsP2.erase(bulletsP2.begin()+i); // iterator helps you quickly access memory locations; this points to first slot, and then you just hop over to yours
        }
    }
    
// criteria for hitting the boss--must be true to do damage
    if (boss.bossOnScreen == true && boss.shieldsUp == false) {
        for (int i = 0; i < bulletsP1.size(); i++) {
            if (ofDist(bulletsP1[i].xPos, bulletsP1[i].yPos, boss.xPos + 170, boss.yPos - 113) < 170) {
                boss.health-=2;
                player1.numKilled+=200;
                bulletsP1.erase(bulletsP1.begin()+i);
            }
        }
        
        for (int i = 0; i < bulletsP2.size(); i++) {
            if (ofDist(bulletsP1[i].xPos, bulletsP1[i].yPos, boss.xPos + 170, boss.yPos - 113) < 170) {
                boss.health--;
                player2.numKilled+=200;
                bulletsP2.erase(bulletsP2.begin()+i);
            }
        }
    }
}

//--------------------------------------------------------------

void testApp::addBullets() {
//where we should be taking our OSC messages and passing them into b.setup() to change stuff.
    // p1
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
    
    // p2
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
    
    
// seeing how different the frequencies are
    int diffBetweenMaxLoc = abs(p1MaxLocForFFT - p2MaxLocForFFT);

// setting up the bullets    
    if (gameState == 1) {
        // p1
        if (player1.hasStartedYelling) {
            if (bulletTimerP1 > bulletTimeP1) {
                bulletTimerP1 = 0; //resetting the timer
                Bullet b;
                b.setup(ofMap(maxLevelP1, 0, .9, 10, 30), p1MaxLocForFFT, diffBetweenMaxLoc, player1.xPos);
                bulletsP1.push_back(b);
            }
        }
        
        //p2
        if (player2.hasStartedYelling) {
            if (bulletTimerP2 > bulletTimeP2) {
                bulletTimerP2 = 0; //resetting the timer
                Bullet b;
                b.setup(ofMap(maxLevelP2, 0, .9, 10, 30), p2MaxLocForFFT, diffBetweenMaxLoc, player2.xPos);
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
//		case 'r':
//			recordContext.toggleRegisterViewport();
//			break;
		default:
			break;
            
        case 'a':
            micSensitivity+=.05;
            break;
            
        case 'z':
            micSensitivity-=.05;
            break;
            
        case 'r':
            reset();
            break;
	}
}

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
    
    numLevelsToStore = 50;

    micSensitivity = .3;
    
    shakeSensitivity = micSensitivity*1.2;
    maxShake = 5;
    
    // calling text file for waves of enemies
    loadFromText();
    
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
