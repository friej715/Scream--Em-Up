#include "testApp.h"

/*
 to-do:
 - on/off the ellipse
 
 ** map to keyboard the threshold for imaging (how different something has to be from the bg image; higher means a lot)
 ** map mic sensitivity to keyboard
 ** make it easier to switch between mic and osc
 
 */


//--------------------------------------------------------------
void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	// enable depth->video image calibration
	kinect.setRegistration(true);
    
	kinect.init();
	//kinect.init(false, false); // disable video image (faster fps)
	kinect.open();
	
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
    
    grayDiff.allocate(kinect.width, kinect.height);
    grayBg.allocate(kinect.width, kinect.height);
	
	ofSetFrameRate(60);
	
	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);
    
    bLearnBackground = true;
    
    // threshold for more accurate blob detection
    threshold = 80;
    
    //setting up the audio
    ofSoundStreamSetup(0, 2, this, 44100, 256, 4);
    
    //game stuff!----------------------
    
    bulletTimerP1 = 0;
    bulletTimeP1 = 2; //frames
    
    bulletTimerP2 = 0;
    bulletTimeP2 = 2; //frames
    
    enemiesShot = 0;
    
    gameScale = ofGetScreenWidth()/kinect.width;
    
    numLevelsToStore = 50;
    
    micSensitivity = .15;
    
    inputBufferCopy = new float[512*2];
    
    oldYell.push_back(0.0f);
    
    shakeSensitivity = micSensitivity*1.2;
    maxShake = 20;
    
    // OSC port stuff--will need 2 for 2 players, duh
    receiverP1.setup(8000);
    receiverP2.setup(8001);
    
    grayImageOn = true;
    
    // calling text file for waves of enemies
    loadFromText();
    
    gameStartTime = ofGetElapsedTimeMillis();
    
    // gamestate
    gameState = 1;
    
    
    enemyTimer = 0;
    enemyTime = 30;
}

//--------------------------------------------------------------
void testApp::update() {
    bulletTimerP1++; // this can always go up without a problem
    bulletTimerP2++; // bullet timer
    enemyTimer++; // timer for enemies; only relevant if not using text file
    
	kinect.update(); // updating kinect, obvs
    
    
    checkIfYelling(); // ALWAYS want to check if yelling. it's how we control anything, including starting, restarting, etc.
    
    
    // ----- titlescreen -----
    if (gameState == 0) {
        // cout << (maxLevel/micSensitivity)*100 << endl;
        if ((maxLevelP1 > micSensitivity && maxLevelP2 > micSensitivity) || maxLevel > micSensitivity) {
            gameState = 1;
        }
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
                e.setup(enemyXPos[0], enemyYPos[0]);
                enemies.push_back(e);
                // delete from vector
                enemyXPos.erase(enemyXPos.begin());
                enemyYPos.erase(enemyYPos.begin());
                enemyTimeSpawn.erase(enemyTimeSpawn.begin());
            }
        }
        
        // there is a new frame and we are connected
        if(kinect.isFrameNew()) { // we only really care about this if we're in gamestate 1
            
            // load grayscale depth image from the kinect source
            grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
            grayImage.mirror(false, true);
            
            // we do two thresholds - one for the far plane and one for the near plane
            // we then do a cvAnd to get the pixels which are a union of the two thresholds
            
            // update the cv images
            grayImage.flagImageChanged();
            
            if (bLearnBackground) {
                grayBg = grayImage;
                bLearnBackground = false;
            }
            
            grayDiff.absDiff(grayBg, grayImage);
            grayDiff.threshold(threshold);
            
            
            // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
            // also, find holes is set to true so we will get interior contours as well....
            contourFinder.findContours(grayDiff, 10, (kinect.width*kinect.height)/2, 1, false);
            
        }
        
        addBullets(); // adding bullets when yelling; moved to a function because unlikely to change much in this context
        checkBullets(); // checking bullets; moved to a function because we won't really change it often
        
        // update our enemies
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].update();
        }
        
        // different settings for different blobs "found." really how many blobs are found is set by the blobfinder itself (1, 2)
        if (contourFinder.nBlobs == 2) {
            player1.xPos = contourFinder.blobs[0].centroid.x * gameScale;
            player2.xPos = (contourFinder.blobs[1].centroid.x) * gameScale;
        } 
        if (contourFinder.nBlobs == 1) {
            player1.xPos = contourFinder.blobs[0].centroid.x * gameScale;
        }
        if (contourFinder.nBlobs == 0) {
            player1.xPos = 50;
            player2.xPos = 500;
        }
        
        // filling up the yells with the various volumes
        if (player1.hasStartedYelling == true) { //player is currently yelling.
            newYellP1.push_back(volumeP1);
        }
        
        if (player2.hasStartedYelling == true) {
            newYellP2.push_back(volumeP2);
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
    
    cout << "old yell max " << oldYellMax << endl;
    cout << "new yell max p1 " << newYellMaxP1 << endl;
    cout << "new yell max p2 " << newYellMaxP2 << endl;
    
    if (newYellMaxP1 > oldYellMax) {
        // can escalate by moving the above code into here. that way you always have to top the loudest
        oldYell.clear();
        for (int i = 0; i < newYellP1.size(); i++) {
            oldYell.push_back(newYellP1[i]);
        }
        newYellP1.clear();
        player1.isWinning = true;
        player2.isWinning = false;
        cout << "hooray P1" << endl;
        return true;
    } else if (newYellMaxP2 > oldYellMax) {
        oldYell.clear();
        for (int i = 0; i < newYellP2.size(); i++) {
            oldYell.push_back(newYellP2[i]);
        }
        newYellP2.clear();
        player2.isWinning = true;
        player1.isWinning = false;
        cout << "hooray P2" << endl;
        return true;
    } else {
        cout << "boo" << endl;
        return false;
        
    }
    
}

//--------------------------------------------------------------
void testApp::draw() {
    // ----- titlescreen -----
    if (gameState == 0) {
        // obvs change this to the osc messages
        
        float shake = 0;
        shake = ofMap(maxLevel, 0, .1, 0, 15);
//        cout << shake << endl;
        
        ofPushMatrix();
        ofTranslate(ofRandom(shake), ofRandom(shake));
        ofDrawBitmapString("YOU MUST BE THIS FUCKING LOUD\nTO PLAY THIS GAME", ofGetWidth()/2.0, ofGetHeight()/2.0);
        ofLine(0, ofGetHeight() - 500, ofGetWidth(), ofGetHeight()-500);
        ofRect(ofGetWidth()/4.0, ofGetHeight(), 50, -ofMap(maxLevel, 0, .1, 0, 500));
        ofRect(ofGetWidth()*.75, ofGetHeight(), 50, -ofMap(maxLevel, 0, .1, 0, 500));
        ofPopMatrix();
    }
    
    if (gameState == 1) {
        //    ofBackground(255, 192, 203);
        float shake = 0;
        if (maxLevelP1 > shakeSensitivity) {
            shake = ofMap(maxLevelP1, shakeSensitivity, .5, 0, maxShake); 
        } else if (maxLevelP2 > shakeSensitivity) {
            shake = ofMap(maxLevelP2, shakeSensitivity, .5, 0, maxShake);
        }
        
        ofPushMatrix();
        ofTranslate(ofRandom(shake), ofRandom(shake));
        ofSetColor(255, 255, 255);
        
        
        if (grayImageOn) {
            grayDiff.draw(0, 0, grayDiff.width*gameScale, grayDiff.height*gameScale);
        }
        
        //game stuff! 
        
        // let's print the ratio of enemies shot to total enemies on screen.
        ofDrawBitmapString("Player 1: " + ofToString(player1.numKilled), ofGetWidth()/4, 50);
        ofDrawBitmapString("Player 2: " + ofToString(player2.numKilled), ofGetWidth()*.75, 50);
        
        if (player1.isWinning) {
            ofDrawBitmapString("Player 1 is louder!", ofGetWidth()/(ofRandom(1.98,2)), 100);
        }
        
        if (player2.isWinning) {
            ofDrawBitmapString("Player 2 is louder!", ofGetWidth()/(ofRandom(1.98,2)), 100);
        }
        
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
        ofPopMatrix();
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
void testApp::exit() {
    kinect.setCameraTiltAngle(0); // zero the tilt on exit
    kinect.close();
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
                player1.numKilled++;
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
}

//--------------------------------------------------------------

void testApp::addBullets() {
    // this should be where we should be taking our OSC messages and passing them into b.setup() to change stuff.
    
    // so let's get our osc messages first.
    while (receiverP1.hasWaitingMessages()) {
        ofxOscMessage mP1;
        receiverP1.getNextMessage(&mP1);
        // and now we get the volume from MAH APP
        if (mP1.getAddress() == "/volume/max") {
            volumeP1=mP1.getArgAsFloat(0);
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
        ofxOscMessage mP2;
        receiverP2.getNextMessage(&mP2);
        // and now we get the volume from Kampo
        if (mP2.getAddress() == "/volume/match") {
            volumeP2=mP2.getArgAsFloat(0);
            maxLevelsP2.push_back(volumeP2);
            if (maxLevelsP2.size()>numLevelsToStore) {
                maxLevelsP2.erase(maxLevelsP2.begin());
            }
        } else if (mP2.getAddress() == "/test/levels") {
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
    
    // and now we'll pass the f out of this s to the bullet setup function.
//    if (player1.hasStartedYelling) {
        if (bulletTimerP1 > bulletTimeP1) {
            bulletTimerP1 = 0; //resetting the timer
            Bullet b;
            b.setup(ofMap(maxLevelP1, 0, .3, 5, 10), p1MaxLocForFFT, diffBetweenMaxLoc, player1.xPos);
            bulletsP1.push_back(b);
        }
//    }
    
//    if (player2.hasStartedYelling) {
        if (bulletTimerP2 > bulletTimeP2) {
            bulletTimerP2 = 0; //resetting the timer
            Bullet b;
            b.setup(ofMap(maxLevelP2, 0, .1, 5, 10), p2MaxLocForFFT, diffBetweenMaxLoc, player2.xPos);
            bulletsP2.push_back(b);
        }
//    }
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
    switch (key) {
        case ' ':
            bLearnBackground = true;
            //bThreshWithOpenCV = !bThreshWithOpenCV;
            break;
            
        case 'w':
            kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
            break;
            
        case 'o':
            kinect.setCameraTiltAngle(angle); // go back to prev tilt
            kinect.open();
            break;
            
        case 'c':
            kinect.setCameraTiltAngle(0); // zero the tilt
            kinect.close();
            break;
            
        case OF_KEY_UP:
            angle++;
            if(angle>30) angle=30;
            kinect.setCameraTiltAngle(angle);
            break;
            
        case OF_KEY_DOWN:
            angle--;
            if(angle<-30) angle=-30;
            kinect.setCameraTiltAngle(angle);
            break;
            
        case '=':
            threshold++;
            break;
            
        case '-':
            threshold--;
            break;
            
        case 'a':
            micSensitivity+=.05;
            break;
            
        case 'z':
            micSensitivity-=.05;
            break;
            
        case 'r':
            oldYell.clear();
            player1.isWinning = false;
            player2.isWinning = false;
            break;
            
        case 'd':
            grayImageOn = !grayImageOn;
            break;
            
    }
}
//--------------------------------------------------------------
void testApp::loadFromText() {
    // clear vectors to make it not fuck up in between games/quit games
    enemyXPos.clear();
    enemyYPos.clear();
    enemyTimeSpawn.clear();
    
    
    ifstream fin; // input file stream; file in
    vector<string> enemyLines; // vector holding information
    
    fin.open(ofToDataPath("enemies.txt").c_str()); // of looks at bin by default; c-style string (whole thing is like chars)
    
    while (fin!=NULL) { // as long as it's a) not empty and b) at the end of the file
        
        string textFromLine; // temporary string to hold...
        getline(fin, textFromLine); // get the line from fin and store it in temp variable textfromline
        enemyLines.push_back(textFromLine);
    }
    
    for (int i = 0; i < enemyLines.size(); i+=4) {
        int newX = atoi(enemyLines[i].c_str());
        enemyXPos.push_back(newX);
        
        int newY = atoi(enemyLines[i+1].c_str());
        enemyYPos.push_back(newY);
        
        int newTime = atoi(enemyLines[i+2].c_str());
        enemyTimeSpawn.push_back(newTime);
        cout<<"this foe time: "<<newTime<<endl;
    }
    
    cout<<"done loading"<<endl;
    
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
