#include "testApp.h"

void testApp::closeApp(){

	audioFileReader.close();
	foregroundAudioFileWriter.close();
	backgroundAudioFileWriter.close();
	
	free(current_frame);
	free(foreground_frame);
	free(background_frame);
	free(aligned_buffer);
	
	delete circularRecorder;
	delete plca;
	
	OF_EXIT_APP(0);
}

//--------------------------------------------------------------
void testApp::setup(){
	bSetup			= false;
	bSegmenting		= false;
	string filename;
	if(ofxFileDialogOSX::openFile(filename))
	{
		if (audioFileReader.open(filename)) 
		{
			setupPLCA();
		}
		else 
		{
			printf("[ERROR] Could not open %s for reading.\n", filename.c_str());
			OF_EXIT_APP(0);
		}
	}
	else 
	{
		printf("[ERROR] User canceled opening file.\n");
		OF_EXIT_APP(0);
	}
	
	ofSetVerticalSync(false);
	ofSetWindowShape(1000, 520);
	ofSetFrameRate(500);
	
}

void testApp::setupPLCA()
{
	plca_image_width		= 1000;
	plca_image_height		= 120;
	frame					= 0;
	frameSize				= 2048;
	sampleRate				= 44100;
	totalFrames				= audioFileReader.mNumSamples / frameSize / 2;
	numModelFrames			= sampleRate / frameSize * 0.5;
	modelSize				= frameSize * numModelFrames;
	
	circularRecorder		= new pkmCircularRecorder(modelSize,		// size of buffer
													  frameSize);		// frame size (each audio copy)
	current_frame			= (float *)malloc(sizeof(float)*frameSize);
	aligned_buffer			= (float *)malloc(sizeof(float)*modelSize);	// original audio
	foreground_frame		= (float *)malloc(sizeof(float)*modelSize);	// extracted foreground
	background_frame		= (float *)malloc(sizeof(float)*modelSize);	// extracted background
	
	foregroundComponents	= 6;
	backgroundComponents	= 6;
	foregroundIterations	= 50;
	backgroundIterations	= 50;
	
	foregroundAudioFileWriter.open(ofToDataPath("foreground.wav"), frameSize);
	backgroundAudioFileWriter.open(ofToDataPath("background.wav"), frameSize);
	
	// initialize the background model of the plca to the first few frames
	while (frame < numModelFrames*4) {
		audioFileReader.read(current_frame, (frame)*frameSize, frameSize, sampleRate);
		circularRecorder->insertFrame(current_frame);
		audioSegmenter.audioReceived(current_frame, frameSize, 1);
		frame++;
	}
	circularRecorder->copyAlignedData(aligned_buffer);
	
	plca = new pkmPLCA(frameSize, foregroundComponents, backgroundComponents, numModelFrames*frameSize);
	plca->computeBackgroundFromBuffer(aligned_buffer, modelSize, backgroundComponents, backgroundIterations);
	
	//circularRecorder->clear();
	frame = 0;
	bSetup = true;
	
}

//--------------------------------------------------------------
void testApp::update(){
	if(!bSetup)
	{
		return;
	}
	
	
	// get the current audio input frame
	audioFileReader.read(current_frame, frame*frameSize, frameSize, sampleRate);	
	circularRecorder->insertFrame(current_frame);
	circularRecorder->copyAlignedData(aligned_buffer);
	
	// process for segments
	audioSegmenter.audioReceived(current_frame, frameSize, 1);
	// starting to segment
	if (audioSegmenter.isSegmenting() && !bSegmenting) {
		// recompute background
		plca->computeBackgroundFromBuffer(aligned_buffer, modelSize, backgroundComponents, backgroundIterations);
		bSegmenting = true;
		printf("[OK] Detected segment\n");
	}
	// end of segment
	if (audioSegmenter.update())
	{
		audioSegmenter.resetSegment();
		bSegmenting = false;
	}
	
	// compute foreground
	plca->computeForegroundFromBuffer(aligned_buffer, modelSize, foregroundComponents, foregroundIterations);
	
	// extract components
	vDSP_vclr(foreground_frame, 1, modelSize);
	vDSP_vclr(background_frame, 1, modelSize);
	plca->backgroundSubtraction(foreground_frame, background_frame);
	
	plca->updateImages();
	
	// clip output
	float low = -1.0f;
	float high = 1.0f;
	vDSP_vclip(foreground_frame + modelSize - frameSize*2, 1, &low, &high, 
			   foreground_frame + modelSize - frameSize*2, 1, frameSize);
	vDSP_vclip(background_frame + modelSize - frameSize*2, 1, &low, &high, 
			   background_frame + modelSize - frameSize*2, 1, frameSize);
	
	// write the audio out
	foregroundAudioFileWriter.write(foreground_frame + modelSize - frameSize*2, frame*frameSize, frameSize);
	backgroundAudioFileWriter.write(background_frame + modelSize - frameSize*2, frame*frameSize, frameSize);
	
	// update to next audio frame
	frame++;
	if (frame >= totalFrames) {
		printf("[OK] Finished Reading File");
		closeApp();
	}
}


//--------------------------------------------------------------
void testApp::draw() {
	if(!bSetup)
		return;
	
	char buf[256];
	
	ofBackground(0);
	ofSetColor(255, 255, 255);
	ofPushMatrix();
	ofTranslate(0, 15, 0);
	
	if (plca->isReady()) 
	{
		//plca->updateImages();
		
		ofFill();
		ofSetColor(255,255,255);
		
		plca->signalImg.draw(0, 0, plca_image_width, plca_image_height);
		sprintf(buf, "ORIGINAL");
		ofDrawBitmapString(buf, 20, plca_image_height + 5);
		
		
		ofLine(0, 135, plca_image_width, 135);
		
		
		plca->foreImg.draw(0, 150, plca_image_width, plca_image_height);
		plca->forePw.draw(20, 170, 80, 80);
		plca->forePh.draw(120, 170, 80, 80);
		plca->forePz.draw(220, 170, 80, 80);
		sprintf(buf, "FOREGROUND");
		ofDrawBitmapString(buf, 20, plca_image_height + 155);
		
		
		ofLine(0, 135 + 150, plca_image_width, 135 + 150);
		
		
		plca->backImg.draw(0, 300, plca_image_width, plca_image_height);
		plca->backPw.draw(20, 320, 80, 80);
		plca->backPh.draw(120, 320, 80, 80);
		plca->backPz.draw(220, 320, 80, 80);
		sprintf(buf, "BACKGROUND");
		ofDrawBitmapString(buf, 20, plca_image_height + 305);
		
		ofLine(0, 135 + 300, plca_image_width, 135 + 300);
		
		ofNoFill();
		ofRect(20, 170, 80, 80);
		ofRect(120, 170, 80, 80);
		ofRect(220, 170, 80, 80);
		ofRect(220, 320, 80, 80);
		ofRect(120, 320, 80, 80);
		ofRect(20, 320, 80, 80);

		
		
	}
	
	ofPopMatrix();
	
	float fps = ofGetFrameRate();
	sprintf(buf, "fps: %02.2f", fps);
	ofDrawBitmapString(buf, ofPoint(20,470));
	sprintf(buf, "processed: %d/%d", frame, totalFrames);
	ofDrawBitmapString(buf, ofPoint(20,485));
	
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	if(key == 'q')
		closeApp();
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::resized(int w, int h){

}

