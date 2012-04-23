#include "testApp.h"

/*
 to-do:
 - fix the drawing/scaling
 - on/off the ellipse
 - add enemies!
 
 
 
 
 
 */


//--------------------------------------------------------------
void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	// enable depth->video image calibration
	kinect.setRegistration(true);
    
	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)
	kinect.open();
	
#ifdef USE_TWO_KINECTS
	kinect2.init();
	kinect2.open();
#endif
	
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
    
    grayDiff.allocate(kinect.width, kinect.height);
    grayBg.allocate(kinect.width, kinect.height);
	
	ofSetFrameRate(60);
	
	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);
	
	// start from the front
	bDrawPointCloud = false;
    
    bLearnBackground = true;

    // threshold for more accurate blob detection
    threshold = 80;
    
    //setting up the audio
    ofSoundStreamSetup(0, 2, this, 44100, 256, 4);
}

//--------------------------------------------------------------
void testApp::update() {
	
	ofBackground(100, 100, 100);
	
	kinect.update();
	
    
    
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		
		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
		
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
	
#ifdef USE_TWO_KINECTS
	kinect2.update();
#endif
}

//--------------------------------------------------------------
void testApp::draw() {
	
	ofSetColor(255, 255, 255);
	
	if(bDrawPointCloud) {
		easyCam.begin();
		drawPointCloud();
		easyCam.end();
	} else {
		// draw from the live kinect
		kinect.drawDepth(10, 10, 400, 300);
		grayBg.draw(420, 10, 400, 300);
    
		grayImage.draw(10, 320, 400, 300);
		grayDiff.draw(420, 320, 400, 300);
		
        contourFinder.draw(420, 320, 400, 300);
        
        for (int i = 0; i < contourFinder.nBlobs; i++) {
            ofFill();
            ofSetColor(255, 0, 0);
            // this will be changed to use a binary threshold instead (on/off)
            ofEllipse(contourFinder.blobs[i].centroid.x, contourFinder.blobs[i].centroid.y, ofMap(maxLevel, 0, .1, 20, 50), ofMap(maxLevel, 0, .1, 20, 50));
            cout << contourFinder.blobs[i].centroid.x << endl;
        }

        
#ifdef USE_TWO_KINECTS
		kinect2.draw(420, 320, 400, 300);
#endif
	}

}

void testApp::drawPointCloud() {
	int w = 640;
	int h = 480;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_POINTS);
	int step = 2;
	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			if(kinect.getDistanceAt(x, y) > 0) {
				mesh.addColor(kinect.getColorAt(x,y));
				mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
			}
		}
	}
	glPointSize(3);
	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards' 
	ofScale(1, -1, -1);
	ofTranslate(0, 0, -1000); // center the points a bit
	glEnable(GL_DEPTH_TEST);
	mesh.drawVertices();
	glDisable(GL_DEPTH_TEST);
	ofPopMatrix();
}

//--------------------------------------------------------------
void testApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
	
#ifdef USE_TWO_KINECTS
	kinect2.close();
#endif
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
void testApp::keyPressed (int key) {
	switch (key) {
		case ' ':
			bLearnBackground = true;
            //bThreshWithOpenCV = !bThreshWithOpenCV;
			break;
			
		case'p':
			bDrawPointCloud = !bDrawPointCloud;
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
	}
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
