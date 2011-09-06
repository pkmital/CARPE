#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxFileDialogOSX.h"
#include "pkmEXTAudioFileReader.h"
#include "pkmEXTAudioFileWriter.h"
#include "pkmAudioSegmenter.h"
#include "pkmCircularRecorder.h"
#include "pkmPLCA.h"
#include "ofxOpenCv.h"

class testApp : public ofBaseApp{

	public:

	void closeApp();
	void setupPLCA();
	void setup();
	void update();
	void draw();

	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void resized(int w, int h);

	
	pkmEXTAudioFileReader		audioFileReader;
	pkmEXTAudioFileWriter		foregroundAudioFileWriter, backgroundAudioFileWriter;
	
	pkmPLCA						*plca;
	pkmAudioSegmenter			audioSegmenter;
	pkmCircularRecorder			*circularRecorder;
    
    ofVideoPlayer               vidPlayer;
    ofxCvColorImage             colorImg;
    ofxCvGrayscaleImage         grayImg, pGrayImg;
    ofxCvColorImage             saliencyImg;
	
	int							plca_image_width, plca_image_height;
	int							foregroundComponents, backgroundComponents, foregroundIterations, backgroundIterations;
	int							frame, frameSize, sampleRate, totalFrames, numModelFrames, modelSize;
	
	float						*current_frame, *foreground_frame, *background_frame, *aligned_buffer;

	bool						bSetup, bSegmenting;
};

#endif
