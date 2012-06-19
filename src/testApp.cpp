#include "testApp.h"

/*
 to-do:
 */


//--------------------------------------------------------------
void testApp::setup() {
    
    titleFont.loadFont("arcade.ttf", 72);
    scoreFont.loadFont("arcade.ttf", 30);
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
    
    // OSC port stuff--will need 2 for 2 players, duh
    receiverP1.setup(8014);
    receiverP2.setup(8015);
    
    bLearnBackground = true;
    
    // threshold for more accurate blob detection
    threshold = 80;
    
    //setting up the audio
    ofSoundStreamSetup(0, 2, this, 44100, 256, 4);
    
//    //game stuff!----------------------
//    
//    bulletTimerP1 = 0;
//    bulletTimeP1 = 2; //frames
//    
//    bulletTimerP2 = 0;
//    bulletTimeP2 = 2; //frames
//    
//    enemiesShot = 0;
//    
//    gameScale = ofGetScreenWidth()/kinect.width;
//    
//    numLevelsToStore = 50;
//    
////    micSensitivity = .2;
//    micSensitivity = .8;
//    
//    inputBufferCopy = new float[512*2];
//    
//    oldYell.push_back(0.0f);
//    
//    shakeSensitivity = micSensitivity*1.2;
//    maxShake = 5;
//    
//    grayImageOn = true;
//    
//    // calling text file for waves of enemies
//    loadFromText();
//    
//    // gamestate
//    gameState = 0;
//    
//    
//    enemyTimer = 0;
////    enemyTime = 30;
//    enemyTime = 15;
//
//    ofEnableAlphaBlending();
//
//    for (int i = 0; i < 30; i++) {
//        ofVec2f s;
//        s.x = ofRandom(ofGetWidth());
//        s.y = ofRandom(ofGetHeight());
//        regularStars.push_back(s);
//    }
//    
//    boss.setup();
//    gameStartTime = ofGetElapsedTimeMillis();
    
    isUsingKeyboard = false;
    reset();
    
   gameState = 0;
    
    cout << boss.font.stringWidth("F") << endl;
    cout << boss.font.stringHeight("F") << endl;
}

//--------------------------------------------------------------
void testApp::update() {
    bulletTimerP1++; // this can always go up without a problem
    bulletTimerP2++; // bullet timer
    enemyTimer++; // timer for enemies; only relevant if not using text file
    
	kinect.update(); // updating kinect, obvs
    
    
    checkIfYelling(); // ALWAYS want to check if yelling. it's how we control anything, including starting, restarting, etc.
    compareYells();

    
    
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
            contourFinder.findContours(grayDiff, 10, (kinect.width*kinect.height)/2, 2, false);
            
        }
        
        addBullets(); // adding bullets when yelling; moved to a function because unlikely to change much in this context
//        checkBullets(); // checking bullets; moved to a function because we won't really change it often
        
        // update our enemies
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].update();
        }
        
        player1.update();
        player2.update();
        
        
        if (isUsingKeyboard == false) {
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
        
        
        
        ofBackground(0);
        float shake = 0;
        shake = ofMap(maxLevelP1 + maxLevelP2, 0, .6, 0, 10);
        
        ofPushMatrix();
        //ofTranslate(ofRandom(shake), ofRandom(shake));
        
        ofColor c;
        c.setHsb(ofRandom(255), 255, 255, 255);
        ofSetColor(c);
        titleFont.drawString("SCREAM 'EM UP!1", 0, 100);
        
        ofSetColor(255);
        scoreFont.drawString("YOU MUST BE THIS LOUD\n   TO PLAY THIS GAME", 200, 150);
        
        ofSetColor(c);
        ofSetLineWidth(5);
        ofLine(0, 160, ofGetWidth(), 160);
        
        ofRect(ofGetWidth()/4.0, ofGetHeight(), 50, -ofMap(maxLevelP1, 0, micSensitivity, 0, ofGetHeight()-160));
        ofRect(ofGetWidth()*.75, ofGetHeight(), 50, -ofMap(maxLevelP2, 0, micSensitivity, 0, ofGetHeight()-160));
        
        ofPopMatrix();
        
    }
    
    if (gameState == 1) {
        
        grayImageOn;
        
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
        
        
        if (grayImageOn) {
            grayDiff.draw(0, 0, grayDiff.width*gameScale, grayDiff.height*gameScale);
        }
        
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
void testApp::keyPressed (int key) {
    switch (key) {
        case ' ':
            bLearnBackground = true;
            //bThreshWithOpenCV = !bThreshWithOpenCV;
            break;
            
//        case 'w':
//            kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
//            break;
            
//        case 'o':
//            kinect.setCameraTiltAngle(angle); // go back to prev tilt
//            kinect.open();
//            break;
            
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
            
        case 'e':
            oldYell.clear();
            player1.isWinning = false;
            player2.isWinning = false;
            break;
            
        case 'd':
            grayImageOn = !grayImageOn;
            break;
            
        case 'r':
            reset();
            break;
            
        case 'k':
            isUsingKeyboard = !isUsingKeyboard;
            break;
            
    }
    
    if (key == 'q') {
        player1.isMovingLeft = true;
        player1.isMovingRight = false;
    }
    
    if (key == 'w') {
        player1.isMovingRight = true;
        player1.isMovingLeft = false;
    }
    
    if (key == 'o') {
        player2.isMovingLeft = true;
        player2.isMovingRight = false;
    }
    
    if (key == 'p') {
        player2.isMovingRight = true;
        player2.isMovingLeft = false;
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
    
    gameScale = ofGetScreenWidth()/kinect.width;
    
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
