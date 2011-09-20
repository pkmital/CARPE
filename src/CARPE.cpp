// Parag K Mital
/*
 CARPE, The Software" © Parag K Mital, parag@pkmital.com
 
 The Software is and remains the property of Parag K Mital
 ("pkmital") The Licensee will ensure that the Copyright Notice set
 out above appears prominently wherever the Software is used.
 
 The Software is distributed under this Licence: 
 
 - on a non-exclusive basis, 
 
 - solely for non-commercial use in the hope that it will be useful, 
 
 - "AS-IS" and in order for the benefit of its educational and research
 purposes, pkmital makes clear that no condition is made or to be
 implied, nor is any representation or warranty given or to be
 implied, as to (i) the quality, accuracy or reliability of the
 Software; (ii) the suitability of the Software for any particular
 use or for use under any specific conditions; and (iii) whether use
 of the Software will infringe third-party rights.
 
 pkmital disclaims: 
 
 - all responsibility for the use which is made of the Software; and
 
 - any liability for the outcomes arising from using the Software.
 
 The Licensee may make public, results or data obtained from, dependent
 on or arising out of the use of the Software provided that any such
 publication includes a prominent statement identifying the Software as
 the source of the results or the data, including the Copyright Notice
 and stating that the Software has been made available for use by the
 Licensee under licence from pkmital and the Licensee provides a copy of
 any such publication to pkmital.
 
 The Licensee agrees to indemnify pkmital and hold them
 harmless from and against any and all claims, damages and liabilities
 asserted by third parties (including claims for negligence) which
 arise directly or indirectly from the use of the Software or any
 derivative of it or the sale of any products based on the
 Software. The Licensee undertakes to make no liability claim against
 any employee, student, agent or appointee of pkmital, in connection 
 with this Licence or the Software.
 
 
 No part of the Software may be reproduced, modified, transmitted or
 transferred in any form or by any means, electronic or mechanical,
 without the express permission of pkmital. pkmital's permission is not
 required if the said reproduction, modification, transmission or
 transference is done without financial return, the conditions of this
 Licence are imposed upon the receiver of the product, and all original
 and amended source code is included in any transmitted product. You
 may be held legally responsible for any copyright infringement that is
 caused or encouraged by your failure to abide by these terms and
 conditions.
 
 You are not permitted under this Licence to use this Software
 commercially. Use for which any financial return is received shall be
 defined as commercial use, and includes (1) integration of all or part
 of the source code or the Software into a product for sale or license
 by or on behalf of Licensee to third parties or (2) use of the
 Software or any derivative of it for research with the final aim of
 developing software products for sale or license to a third party or
 (3) use of the Software or any derivative of it for research with the
 final aim of developing non-software products for sale or license to a
 third party, or (4) use of the Software to provide any service to an
 external organisation for which payment is received. If you are
 interested in using the Software commercially, please contact pkmital to
 negotiate a licence. Contact details are: parag@pkmital.com
 
 
 *
 *
 */

#include "CARPE.h"
#include "stdio.h"
#include <iostream>
#include <fstream>
#include "myUtils.h"					// open file dialog
#include <opencv2/opencv.hpp>
#include <ofxOpenCv.h>

#include "ofxCvContourFinder.h"
#include "pkmGaussianMixtureModel.h"

#ifndef _USEUNSTABLE
	#include "pkmDROITracker.h"
	#include "pkmBoostingTracker.h"
#else
#endif

#ifdef TARGET_WIN32
#include "Win32InputBox.h"
#include "resource.h"
#include "direct.h"
#else
#include "ofxFileDialogOSX.h"
#include <sys/stat.h>
#endif

#ifndef WITHIN
	#define WITHIN(pt, pt2, radius) (((pt) > ((pt2) - (radius))) && ((pt) < ((pt2) + (radius))))
#endif

int screen_width, screen_height;

//--------------------------------------------------------------
diemDROI::diemDROI(){
}

diemDROI::~diemDROI(){	

#ifndef _USEUNSTABLE
	if(myflowlib)
		delete myflowlib;

	if(frameflowlib)
		delete frameflowlib;

	for(vector<pkmBoostingTracker *>::iterator itr=trackers.begin();
	//for(vector<pkmDROITracker *>::iterator itr=trackers.begin();
        itr!=trackers.end();
        ++itr){
        delete (*itr);
    }
#endif

	delete [] eyeList;
	delete [] xs;					
	delete [] ys;

	if(loadBinocular)
	{
		// binocular storage
		delete [] b_xs;
		delete [] b_ys;
		delete [] m_xs;
		delete [] m_ys;
		// contains both eyes x,y coords
		delete [] inputModelMap;
	}

	delete [] eyePtsMap;

	// for the scaled down colored heatmap
	delete [] unnormalized;
	delete [] colorized;
	delete [] movieOut;

	// the 3 values returned from the colormap conversion
	delete [] rgb;

	// setup asynchronous readback for 2 pbo objects
	glDeleteBuffers(2,pboIds);

	filePtr.close();

	if(!doneRecording && saver.bAmSetupForRecording())
		saver.finishMovie();
}

void diemDROI::initializeOptions()
{	
	// defaults
	showEyes = true;
	showMeanBinocular = true;
	showMovie = true;
#ifndef _USEUNSTABLE
	drawOpticalFlow = false;
#endif
	showHeatmap = true;
	showContours = false;
	showNormalized = true;
	showRealTime = false;
	showRecording = false;
	showClustering = true;
	showAlphaScreen = false;
	showHistogram = false;
	showSaccades = false;
	showSubjectNames = false;
#ifndef _USEUNSTABLE
	showAllFixations = false;
#endif

	loadBinocular = true;

	saveMovieImages = false;
	
	// Movie starts paused?
	isPaused = true;			// mouse click can pause

	map_scalar = 8;
	sigma = 10;
	A =  (1./sqrt(TWO_PI*sigma*sigma));
	B = -(1./(sigma*sigma));
}

void diemDROI::loadEyeTrackingMovie()
{
	//////////////////////////////////////////////////////
	// load a movie
	//////////////////////////////////////////////////////	
	
#ifdef TARGET_WIN32
	movie_name = myOpenMovieDialog();
	if(movie_name.empty())
		OF_EXIT_APP(0);
	ofDisableDataPath();
	mov.loadMovie(movie_name);
	ofEnableDataPath();
#else
	int bOk = ofxFileDialogOSX::openFile(movie_name);
	if( bOk )
	{
		mov.loadMovie(movie_name);
	}	
#endif
	
	// remove the path
#ifdef TARGET_WIN32
	movie_name = movie_name.substr(movie_name.rfind('\\')+1, movie_name.length());
#else
	movie_name = movie_name.substr(movie_name.rfind('/')+1, movie_name.length());
#endif
	// remove the extension
	movie_name = movie_name.substr(0, movie_name.length() - 4);
	
    /*
	movieSpeed = 1.0f;
	mov.setSpeed(movieSpeed);	
	mov.setLoopState(OF_LOOP_NONE);	
	mov.setUseTexture(true);
	mov.play();
	mov.setPaused(true);
	mov.setLoopState(OF_LOOP_NONE);
	frameNumber = 0;
	mov.setFrame(frameNumber);
    */
	
	mov.setLoopState(OF_LOOP_NONE);	
//	mov.setPaused(true);
	frameNumber = 1;
	mov.setFrame(frameNumber);
    
	numFrames = mov.getTotalNumFrames();
	FPS = numFrames/mov.getDuration();
	if(FPS == 0)
	{
		printf("Unable to read FPS from the codec.  Try a different encoding.  Defaulting to 30 FPS...\n");
		FPS = 30;
	}
	ofSetFrameRate(FPS);

	ofSetVerticalSync(true);

	// info for printing text
	movhr = mov.getDuration()/3600.;
	movmin = (int)(mov.getDuration())%3600/60;
	movsec = (int)(mov.getDuration())%60;
	
	// Movie starts paused?
	isPaused = true;			// mouse click can pause
	
	// for the keyboard movie controls, flag when keyboard changes frame
	updatedFrame = false;

	// default eye-tracking setup monitor res
	int screen_width = 1280,
		screen_height = 960;

	// get the movie offset on the original eye-tracking monitor
	offset_x = (screen_width - mov.width) / 2;
	offset_y = (screen_height - mov.height) / 2;

	// SETUP OUTPUT DIRECTORIES
#ifdef TARGET_WIN32
	string direc = string("data\\ias\\") + movie_name;
	mkdir(direc.c_str());
	direc = string("data\\output\\") + movie_name;
	mkdir(direc.c_str());
	direc = string("data\\imgs\\") + movie_name;
	mkdir(direc.c_str());
	direc = string("data\\stats\\") + movie_name;
	mkdir(direc.c_str());
#else
    ofEnableDataPath();
    string direc = ofToDataPath(string("ias/") + movie_name);
	mkdir(direc.c_str(), 0777);
	direc = ofToDataPath(string("output/") + movie_name);
	mkdir(direc.c_str(), 0777);
	direc = ofToDataPath(string("imgs/") + movie_name);
	mkdir(direc.c_str(), 0777);
	direc = ofToDataPath(string("stats/") + movie_name);
	mkdir(direc.c_str(), 0777);
#endif
}

void diemDROI::loadEyeTrackingAudio()
{
	/*
    audioFileReader = new pkmEXTAudioFileReader();
    string movie_audio = "audio/" + movie_name + ".wav";
    audioFileReader->open(ofToDataPath(movie_audio));
    audioFrameSize = 44100/FPS;
	*/
}

void diemDROI::initializeMovieOutput()
{
	saver.listCodecs();
	//saver.setCodecType(20);
	saver.setCodecPhotoJpeg();
	saver.setCodecQualityLevel(OF_QT_SAVER_CODEC_QUALITY_HIGH);

	doneRecording = false;

	//////////////////////////////////////////////////////
	// Output movie to
	//////////////////////////////////////////////////////

	string filename;
	int fileno = 1;
	ifstream in;
	do {
		in.close();
		stringstream str;
		// format the filename here
		if(fileno == 1)
			str << "output/" << movie_name << "/" << movie_name << "_output" << ".mov";
		else
			str << "output/" << movie_name << "/" << movie_name << "_output" << "(" << (fileno-1) << ").mov";
		filename = str.str();
		++fileno; 
		// attempt to open for read
		in.open( ofToDataPath(filename).c_str() );
	} while( in.is_open() );
	in.close();	
	// found a file that does not exists

	// now create the file so that we can start adding frames to it:
	ofstream tmpptr( ofToDataPath(filename).c_str() );
	tmpptr.close();
    
	ofEnableDataPath();
	//string f = "\\data\\output\\" + movie_name + "\\" + filename;
	saver.setup(mov.width, mov.height, ofToDataPath(filename));	

	string out_movie_audio = "audio/" + movie_name + ".wav";
	saver.addAudioTrack(ofToDataPath(out_movie_audio));
	//////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
}

void diemDROI::loadEyeTrackingData()
{
	// get the directory listing of the eye-tracking file directory
	//maxEyeListSize = myDIR.listDir(myOpenFolderDialog().c_str(), movie_name);
	//ofEnableDataPath();
	// get the max number of eye-files for each frame
	myDIR.allowExt("txt");
#ifdef TARGET_WIN32
	maxEyeListSize = myDIR.listDir(getApplicationDirectory() + string("\\data\\event_data"), movie_name);
#else
	ofEnableDataPath();
	maxEyeListSize = myDIR.listDir(ofToDataPath("event_data", true), movie_name);
#endif
	//int manualOverideMaxEyeListSize = 14;		// doing this for edScenes data..
	//maxEyeListSize = MIN(maxEyeListSize, manualOverideMaxEyeListSize);
	
	ofEnableDataPath();

	if(maxEyeListSize == 0)
	{
		printf("Error loading eye tracking files\n");
		OF_EXIT_APP(0);
		return;
	}
	// set both mono and bino to this value.  this may be smaller when there is a blink.
	// because of dynamic storage, this gets messy but should be faster than always resizing stl vectors.	
	eyeListSize = maxEyeListSize;
	if(loadBinocular)
	{
		b_eyeListSize = maxEyeListSize;
	}
	else
	{
		b_eyeListSize = 0;
	}


	// (storage for file ptr, x,y coordinates)
	eyeList = new eye[maxEyeListSize];					 // files

	// monocular storage
	xs = new int[maxEyeListSize];						
	ys = new int[maxEyeListSize];

	if(loadBinocular)
	{
		// binocular storage
		b_xs = new int[maxEyeListSize];
		b_ys = new int[maxEyeListSize];
		m_xs = new int[maxEyeListSize];
		m_ys = new int[maxEyeListSize];
		prev_m_xs = new int[maxEyeListSize];
		prev_m_ys = new int[maxEyeListSize];

		// for scaling the eyes as they stay fixated
		scale_eyes = new float[maxEyeListSize];
		for(int i = 0; i < maxEyeListSize; i++)
		{
			prev_m_xs[i] = 0;
			prev_m_ys[i] = 0;
			scale_eyes[i] = 1.0f;
		}

		// contains both eyes x,y coords
		inputModelMap = new double[maxEyeListSize*4];
	}
	else
	{
		inputModelMap = new double[maxEyeListSize*2];
	}
	eyeCounter = 0;
}


void diemDROI::initializeGui()
{
	//loadFont(ofToDataPath("verdana.ttf"), 10);
	//loadFont(ofToDataPath("automat.ttf"), 10);
	//////////////////////////////////////////////////////	
	// instantiate the gui
	//////////////////////////////////////////////////////	
	gui = ofxGui::Instance(this);

	//panel5 = gui->addPanel(gui_VisualizationPanel, "Visualizations", 300, 20, OFXGUI_PANEL_BORDER, 5);
	//panel5->addButton(gui_DrawOpticalFlowBtn, "Motion", 
	//	OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT,
	//	kofxGui_Button_Off, kofxGui_Button_Switch);
	//panel5->addButton(gui_DrawOpticalFlowBtn, "Regions", 
	//	OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT,
	//	kofxGui_Button_Off, kofxGui_Button_Switch);
	//panel5->addButton(gui_DrawOpticalFlowBtn, "Edges", 
	//	OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT,
	//	kofxGui_Button_Off, kofxGui_Button_Switch);

	// add the objects panel
	panel1 = gui->addPanel(gui_OptionsPanel, "Viewing Options", 
		1, 1, 
		10, 10);
	panel1->addButton(gui_EnableMovie, "Show Movie", 
		10, 10, showMovie, kofxGui_Button_Switch); 
//#ifndef _USEUNSTABLE
//	if(enableMotion)
//	{
		panel1->addButton(gui_DrawOpticalFlowBtn, "Show Motion", 
			10, 10, drawOpticalFlow, kofxGui_Button_Switch);
//	}
//#endif
	panel1->addButton(gui_DrawFlicker, "Show Flicker",
		10, 10, showFlicker, kofxGui_Button_Switch);
	panel1->addButton(gui_DrawEdges, "Show Edges",
		10, 10, showEdges, kofxGui_Button_Switch);
	panel1->addButton(gui_EnableEyes, "Show Eyes", 
		10, 10, showEyes, kofxGui_Button_Switch);  
	panel1->addButton(gui_EnableSaccades, "Show Saccades", 
		10, 10, showSaccades, kofxGui_Button_Switch);  
	panel1->addButton(gui_EnableSubjectNames, "Show Subject Names",
		10, 10, showSubjectNames, kofxGui_Button_Switch);
#ifndef _USEUNSTABLE
	panel1->addButton(gui_EnableAllFixations, "Show Fixation Distribution", 
		10, 10, showAllFixations, kofxGui_Button_Switch);  
#endif
	panel1->addButton(gui_EnableHeatmap, "Show Heatmap", 
		10, 10, showHeatmap, kofxGui_Button_Switch); 
	panel1->addButton(gui_EnableContours, "Show Contours", 
		10, 10, showContours, kofxGui_Button_Switch); 
	panel1->addButton(gui_EnableAlphaScreen, "Show Peekthrough", 
		10, 10, showAlphaScreen, kofxGui_Button_Switch); 
	panel1->addButton(gui_EnableHistogram, "Show RGB Histogram", 
		10, 10, showHistogram, kofxGui_Button_Switch); 
	panel1->addButton(gui_EnableMeanBinocular, "Use Mean Binocular Eyes", 
		10, 10, showMeanBinocular, kofxGui_Button_Switch); 
	panel1->addButton(gui_EnableClustering, "Use Clustering", 
		10, 10, showClustering, kofxGui_Button_Switch);
	panel1->addButton(gui_EnableNormalized, "Normalize Heatmap", 
		10, 10, showNormalized, kofxGui_Button_Switch); 
	panel1->addButton(gui_EnableRealTime, "Real Time Playback", 
		10, 10, showRealTime, kofxGui_Button_Switch); 	
	string cov_tags[] = {"Spherical", "Diagonal", "Generic"};
	cov_type = COV_SPHERE;
	panel1->addSwitch(gui_CovarianceSwitch, "Covariance Type", 
		175, OFXGUI_SLIDER_HEIGHT, 
		cov_type, 2, 0, &cov_tags[0]);
	panel1->addSwitch(gui_ResSwitch, "Monitor Resolution", 
		175, OFXGUI_SLIDER_HEIGHT, 
		0, 5, 3, &res_tags[0]);
	panel1->addSwitch(gui_ScalarsSwitch, "Heatmap Quality", 
		175, OFXGUI_SLIDER_HEIGHT, 
		0, 4, 2, &scalar_tags[0]);
	//const cluster_tags[] = {"1", "2", "4", "6", "8", "1-2", "1-4", "1-8", "2-4", "4-8"};
	panel1->addSwitch(gui_ClustersSwitch, "Clustering Kernels", 
		175, OFXGUI_SLIDER_HEIGHT, 
		0, 9, 7, &cluster_tags[0]);
	panel1->addSlider(gui_EyefilesSwitch, "Number of Subjects",
		175, OFXGUI_SLIDER_HEIGHT,
		0, maxEyeListSize, maxEyeListSize, 1, 1);
	minClusterComponents = 1; 
	maxClusterComponents = 8;
	//panel1->addSwitch(gui_ResSwitch, "Monitor Resolution", 
	//	175, OFXGUI_SLIDER_HEIGHT, 
	//	0, 5, 3, &res_tags[0]);
	panel1->active = false;
	
	panel6 = gui->addPanel(gui_ExportPanel, "Recording",
		1, 485, 
		10, 10);
	panel6->addButton(gui_EnableRecording, "Record Output Movie", 
		10, 10, showRecording, kofxGui_Button_Switch);
	panel6->addButton(gui_EnableSaveMovieImages, "Record Movie Images", 
		10, 10, saveMovieImages, kofxGui_Button_Switch);	
	// record a frame to an image??
	panel6->active = false;


#ifndef _USEUNSTABLE
	panel4 = gui->addPanel(gui_TrackPanel, "dROI Options", 
		mov.width - 156, 10, 
		10, 10);
	trackButton = (ofxGuiButton *)panel4->addButton(gui_TrackObjectBtn, "Track Objects", 
		OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, 
		kofxGui_Button_Off, kofxGui_Button_Switch);
	useAdaptTrack = false;
	if(enableMotion)
	{
		panel4->addButton(gui_TrackFlow, "Motion Tracking", 
			OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, 
			kofxGui_Button_On, kofxGui_Button_Switch);
	}
	panel4->addButton(gui_TrackBoosting, "Classifier Tracking", 
		OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, 
		kofxGui_Button_Off, kofxGui_Button_Switch);
	panel4->addButton(gui_TrackAdaptiveBtn, "Adaptive Updates", 
		OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, 
		kofxGui_Button_Off, kofxGui_Button_Switch);
	binDROIs = false;
	panel4->addButton(gui_BinDROIs, "Bin REC dROIS", 
		OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, 
		kofxGui_Button_Off, kofxGui_Button_Switch);
	panel4->addButton(gui_InitDROIs, "Re-Initialize REC dROIS", 
		OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, 
		kofxGui_Button_Off, kofxGui_Button_Trigger);
	
	//panel4->addButton(gui_TrackCAMShift, "Color Tracking", 
	//	OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, 
	//	kofxGui_Button_Off, kofxGui_Button_Switch);
	panel4->active = false;
	
	tracker_type = dense_optical_flow; // boosting


	// Objects panel
	panel5 = gui->addPanel(gui_ObjectsPanel, "dROI List", 
		mov.width - 156, 165, 
		10, 10);
	panel5->addButton(gui_ImportFromIAS, "Load dROIs from IAS",
		OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT,
		kofxGui_Button_Off, kofxGui_Button_Trigger);
	panel5->addButton(gui_ExportAllObjects, "Save dROIs to IAS",
		OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT,
		kofxGui_Button_Off, kofxGui_Button_Trigger);
	panel5->addButton(gui_NewFHObjectTrigger, "New Freehand dROI", 
		OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, 
		kofxGui_Button_Off, kofxGui_Button_Trigger);
	panel5->addButton(gui_NewRectObjectTrigger, "New Rectangular dROI", 
		OFXGUI_BUTTON_HEIGHT, OFXGUI_BUTTON_HEIGHT, 
		kofxGui_Button_Off, kofxGui_Button_Trigger);
	panel5->active = false;
#endif

	//panel1->addSwitch(gui_SigmaSwitch, "Variance", 
	//	175, OFXGUI_SLIDER_HEIGHT, 
	//	0, 8, 5, &sigma_tags[0]);
	//panel1->addSwitch(gui_ScalarsSwitch, "Variance Scalar", 
	//	175, OFXGUI_SLIDER_HEIGHT, 
	//	0, 8, 4, &scalar_tags[0]);

	// the loading bar 
	panel3 = gui->addPanel(gui_Loader, "",
		20,mov.height/2,
		0, 0);
	panel3->addSlider(gui_LoaderSlider, "", 
		mov.width-40, OFXGUI_SLIDER_HEIGHT+10, 
		0.0f,1.0f, 
		movie_time, kofxGui_Display_Float4, 0.0f);
	panel3->active = true;

	// add another panel for the movie time
	// and a slider depicting the time
	panel2 = gui->addPanel(gui_MovieTimeHolder, "",
						   22,mov.height+16,
						   0, 0);
	panel2->addSlider(gui_MovieTimeSlider, "", 
					  mov.width-22-12, OFXGUI_SLIDER_HEIGHT, 
					  0.0f, 1.0f, 
					  0, kofxGui_Display_Float4, 0.0f);
	panel2->active = false;

	gui->activate(true);		// start drawing the gui
	gui->forceUpdate(true);	

	mouse_state_down = 0;
}

void diemDROI::initializeOpticalFlow(){
    
    //We will start at level 0 (full size image) and go down to level 4 (coarse image 16 times smaller than original)
    //Experiment with these values to see how they affect the flow field as well as calculation time
    int max_level = 8;
    int start_level = 1;
    
    //Two pre and post smoothing steps, should be greater than zero
    int n1 = 3;
    int n2 = 1;
    
    //Smoothing and regularization parameters, experiment but keep them above zero
    float rho = 2;
    float alpha = 1000;
    float sigma = 3.0;
    
   
    // motion
    
    // setup the optical flow library
    motion_img.allocate(mov.width,mov.height);

#ifndef _USEUNSTABLE
    frameflowlib = new GPUFlow(0.5);
    myflowlib = new GPUFlow;
    enableMotion = myflowlib->initOK();
#else
    // Set up VarFlow class
    opticalFlow = new VarFlow(mov.width, mov.height, max_level, start_level, n1, n2, rho, alpha, sigma);
    motion_this_img.allocate(mov.width,mov.height);
    motion_previous_img.allocate(mov.width,mov.height);
    motion_previous_previous_img.allocate(mov.width,mov.height);
    motion_x_img.allocate(mov.width,mov.height);
    motion_y_img.allocate(mov.width,mov.height);
    motion_dist_img.allocate(mov.width,mov.height);

    bSaved = 0;
#endif
    drawOpticalFlow = false;   
}

//--------------------------------------------------------------
void diemDROI::setup(){	 
	bSetup = false;
    
    if(!bSetup)
	{
		
		loadEyeTrackingMovie();
		ofSetLogLevel(OF_LOG_VERBOSE);
		ofBackground(0,0,0);
		ofSetWindowTitle("C.A.R.P.E.");
		initializeOptions();
		
		
            // set the window size to match the movie + 30 for the GUI slider
        screen_width = mov.width;
        screen_height = MAX(550,mov.height+60);
		ofSetWindowShape(screen_width, screen_height);
		
		loadEyeTrackingData();
        initializeOpticalFlow();
		
            /////
            // for visualizations:
       
		
		flicker_this_img.allocate(mov.width,mov.height);
		flicker_prev_img.allocate(mov.width,mov.height);
		flicker_img.allocate(mov.width,mov.height);
		showFlicker = false;
		
		edge_img.allocate(mov.width,mov.height);
		showEdges = false;
		
		
            // Setup parameters for the GUI
		initializeGui();
		
            // counter for numframes from eye tracking file
		numFrames = 0;				
		movie_time = 0;
		
            // storage for the heatmap
		heatmap_tex.allocate(mov.width, mov.height, GL_LUMINANCE);
		eyePtsMap = new unsigned char[(mov.width)*(mov.height)];
		for (int i = 0; i < mov.width/map_scalar; i++){
			for (int j = 0; j < mov.height/map_scalar; j++){
				eyePtsMap[(j*(mov.width/map_scalar)+i)] = 0;	
			}
		}
            // for the scaled down colored heatmap
		unnormalized = new unsigned int[mov.width*mov.height];
		colorized = new unsigned char[mov.width*mov.height*3];
		movieOut = new unsigned char[mov.width*mov.height*3];
		
            // cv image for scaling and drawing the colored heatmap
		heatmap3.allocate(mov.width,mov.height);
		heatmap3.setUseTexture(true);
		
            // for normalization
		red.allocate(mov.width, mov.height);
		green.allocate(mov.width, mov.height);
		blue.allocate(mov.width, mov.height);
		
            // for the contour display from grayscale values
		heatmap.allocate(mov.width,mov.height);
		
            // the 3 values returned from the colormap conversion
		rgb = new unsigned char[3];
		
		loadedFiles = false;		// wait until files are loaded until drawing movie
		
            // setup asynchronous readback for 2 pbo objects
		pboIds = new GLuint[2];
		pboIds[0] = 0; pboIds[1] = 0;
		const int DATA_SIZE = screen_width*screen_height*4;
		const int PBO_COUNT = 2;
		glGenBuffersARB(PBO_COUNT, pboIds);
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[0]);
		glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, DATA_SIZE, 0, GL_STREAM_READ_ARB);
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[1]);
		glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, DATA_SIZE, 0, GL_STREAM_READ_ARB);
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
		
		movieAndAlpha.allocate(mov.width,mov.height,GL_RGBA);
		alphaScreen.allocate(mov.width,mov.height,GL_ALPHA);
		
		
            // opencv descriptor image for the movie frame (used for histogram analysis)
		movCvImg.allocate(mov.width,mov.height);
		
		saverImg.allocate(mov.width,mov.height,OF_IMAGE_COLOR);
		
		string filename;
		int fileno = 1;
		ifstream in;
		do {
			in.close();
			stringstream str;
                // format the filename here
			if(fileno == 1)
				str << "data/stats/" << movie_name << "/" << movie_name << "_stats" << ".txt";
			else
				str << "data/stats/" << movie_name << "/" << movie_name << "_stats" << "(" << (fileno-1) << ").txt";
			filename = str.str();
			++fileno; 
                // attempt to open for read
			in.open( filename.c_str());
		} while( in.is_open() );
		in.close();	
            // found a file that does not exists
		
            // now create the file so that we can start adding txt to it:
		ofstream tmpptr(filename.c_str());
		tmpptr.close();
		
            //string out_file = "data/stats/" + movie_name + "_stats.txt";
		filePtr.open(filename.c_str());
		
		
#ifndef _USEUNSTABLE
            ///// SETUP the different trackers
            // setup the objects list
		current_obj = -1;
		trackCurrentObject = false;
		loading_obj = false;
#endif
		
		bSetup = true;
	}	
}
//--------------------------------------------------------------
void diemDROI::update(){
	
	
	
	//if((!isPaused || updatedFrame) && trackCurrentObject && current_obj != -1)
	//{
	//	updatedFrame = false;
	//	// note this automatically updates the movie time to the next frame
	//	trackObject();
	//
	//}
	//else if((!isPaused || updatedFrame) && !trackCurrentObject)
	//{		
	//	updatedFrame = false;
	//	mov.idleMovie();
	//}
	//
	//frameNumber = mov.getCurrentFrame();
	//movie_time = mov.getPosition();

	//// update the movie slider to reflect the current movie time
	//gui->update(gui_MovieTimeSlider, kofxGui_Set_Float, 
	//	&movie_time, sizeof(float));

	//if(current_obj != -1)
	//{
	//	objs[current_obj]->setFrame(frameNumber);
	//}




	// if we have loaded the eye-tracking files, proceed to 
	// display the movie and eye-stuff
	if(loadedFiles)
	{
		// if we have already looped through the film once, then save the movie		
		// else update the movie
		//if(mov.getIsMovieDone() || frameNumber >= mov.getTotalNumFrames())
		if(frameNumber >= numFrames)
		{
			saver.finishMovie();
			doneRecording = true;
			isPaused = true;
			mov.setPaused(isPaused);
		}
		else
		{
			if(showRealTime)
			{

#ifndef _USEUNSTABLE
				if((!isPaused || updatedFrame) & trackCurrentObject && current_obj != -1)
				{
					updatedFrame = false;
					// note this automatically updates the movie time to the next frame
					trackObject();
					frameNumber = mov.getCurrentFrame();
				
				}
				else if((!isPaused || updatedFrame) && !trackCurrentObject)
				{		
					updatedFrame = false;
					mov.update();
					frameNumber = mov.getCurrentFrame();
				}
#else
					updatedFrame = false;
					mov.update();
					frameNumber = mov.getCurrentFrame();
#endif

			}
			else
			{		
                //mov.setPosition(frameNumber / (float)numFrames);
                mov.setFrame(frameNumber);
                mov.idleMovie();
                    //mov.update();
				if(!isPaused)
				{
					frameNumber++;
				}
#ifndef _USEUNSTABLE
				if((!isPaused || updatedFrame) && trackCurrentObject && current_obj != -1)
				{
					updatedFrame = false;
					// note this automatically updates the movie time to the next frame
					trackObject();
					//frameNumber = mov.getCurrentFrame();
				
				}
				else if((!isPaused || updatedFrame) && !trackCurrentObject)
				{		
					updatedFrame = false;
				}
#else
					updatedFrame = false;
#endif
			}

#ifndef _USEUNSTABLE
			for(vector<pkmDROI *>::iterator itr=objs.begin(); itr!=objs.end(); ++itr)
			{
				(*itr)->updateTo(frameNumber);
				if(binDROIs)
					(*itr)->clearCurrentFrame();
			}
#endif
		}

		// if the movie is paused, then only update if the user has manually changed the movie time, in which case,
		// the last frameNumber should be different than the current frameNumber...
		if((lastFrameNumber + 1) != frameNumber || !isPaused || (reset && frameNumber > 1))
		{
			reset = false;
			lastFrameNumber = frameNumber - 1;
			// get the position (0.0f-1.0f)
			movie_time = mov.getPosition();
			// update the movie slider to reflect the current movie time
			gui->update(gui_MovieTimeSlider, kofxGui_Set_Float, 
				&movie_time, sizeof(float));

			//printf("frame #: %d\n", (int)(movie_time*(float)(numFrames-1.)));
			
			if(drawOpticalFlow)
				updateColorFlow();
			if(showFlicker)
				updateFlicker();
			if(showEdges)
				updateEdges();

			updateEyesForCurrentFrame();

			double dist;
			int x,y,i;
			unsigned char r,g,b;
			unsigned long total = 0;
			double max = 0.0000001;
			
			if(showHeatmap || showContours || showAlphaScreen || showClustering)
			{			
				int width = mov.width / map_scalar;
				int height = mov.height / map_scalar;

				// calculate the gaussian distances from each eye pt and 
				// add to a cummulatice distance field, eyePtsMap
				// this is used for both the heatmap and the contour analysis
				// to reduce computation time, the map is reduced by a factor of
				// map_scalar in each direction.  this is set initially in the 
				// applciation constructor.

                /*
				for (x = 0; x < mov.width; x++)	
				{
					for (y = 0; y < mov.height; y++)	
					{
						eyePtsMap[y*mov.width+x] = 0;
						colorized[(x+y*(mov.width))*3+0] = 0;
						colorized[(x+y*(mov.width))*3+1] = 0;
						colorized[(x+y*(mov.width))*3+2] = 0;
					}
                 }*/
                memset(eyePtsMap, 0, sizeof(unsigned char)*mov.width*mov.height);
                memset(colorized, 0, sizeof(unsigned char)*mov.width*mov.height*3);

				if (showClustering && ((eyeListSize+b_eyeListSize) > 1))
				{
					if(showMeanBinocular)
					{
						pkmGaussianMixtureModel myModel(inputModelMap, m_eyeListSize, 2, map_scalar, cov_type);
						myModel.modelData(minClusterComponents, maxClusterComponents, 0, 0);
						//filePtr << (int)(movie_time*(float)(numFrames-1.)) << "\n";
						filePtr << frameNumber << "\n";
						myModel.getLikelihoodMap(height, width, eyePtsMap, filePtr);
					}
					else
					{
						pkmGaussianMixtureModel myModel(inputModelMap, (eyeListSize+b_eyeListSize), 2, map_scalar, cov_type);
						myModel.modelData(minClusterComponents, maxClusterComponents, 0, 0);
						//filePtr << (int)(movie_time*(float)(numFrames-1.)) << "\n";
						filePtr << frameNumber << "\n";
						myModel.getLikelihoodMap(height, width, eyePtsMap, filePtr);
					}

					//myModel.writeToFile(filePtr, true, false, true, true, true);
				}

				if(showHeatmap || showContours || showAlphaScreen)
				{

					for (x = 0; x < width; x++)	
					{
						for (y = 0; y < height; y++)	
						{
							if(!showClustering)
							{
								// if we aren't doing the gaussian mixture model, then we create the model from 
								// the simple assumption that each x,y is it's own gaussian and set unnormalized[] to 
								// the sum of all gaussians
								dist = 0;
								if(!showMeanBinocular)
								{
	#pragma omp parallel for reduction(+:dist)
									for (i = 0; i < eyeListSize; i++)
									{
										//dist += gaussDist(x,(xs[i]/map_scalar),8)*gaussDist(y,(ys[i]/map_scalar),8)*65536;
										dist += gaussDist(x,(xs[i]/map_scalar))*gaussDist(y,(ys[i]/map_scalar))*65536.;
									}
									if(loadBinocular)
									{
	#pragma omp parallel for reduction(+:dist)
										for (i = 0; i < b_eyeListSize; i++)
										{
											dist += gaussDist(x,(b_xs[i]/map_scalar))*gaussDist(y,(b_ys[i]/map_scalar))*65536.;
										}
									}
								}
								else
								{
	#pragma omp parallel for reduction(+:dist)
									for (i = 0; i < m_eyeListSize; i++)
									{
										//dist += gaussDist(x,(xs[i]/map_scalar),8)*gaussDist(y,(ys[i]/map_scalar),8)*65536;
										dist += gaussDist(x,(m_xs[i]/map_scalar))*gaussDist(y,(m_ys[i]/map_scalar))*65536.;
									}
								}

								max = max > dist ? max : dist;
								unnormalized[x+y*width] = dist;
							}
							else
							{
								dist = eyePtsMap[(x+y*width)];
								unnormalized[x+y*width] = dist;
								max = max > dist ? max : dist;
							}

							// map the gray values to the rgb domain
							jetColorMap(rgb,dist,0,255);
							colorized[(x+y*(mov.width))*3+0] = rgb[0];
							colorized[(x+y*(mov.width))*3+1] = rgb[1];
							colorized[(x+y*(mov.width))*3+2] = rgb[2];

						}
					}

					if(showNormalized)
					{
	#pragma omp parallel for private(x,y,dist) shared(width,height)
						for (x = 0; x < width; x++)	
						{
							for (y = 0; y < height; y++)	
							{
	#pragma omp critical
								{
								int sub = (float)unnormalized[x+y*width] / (float)max * 255.0f;
								jetColorMap(rgb,sub,0,255);
								colorized[(x+y*mov.width)*3+0] = rgb[0];
								colorized[(x+y*mov.width)*3+1] = rgb[1];
								colorized[(x+y*mov.width)*3+2] = rgb[2];
								eyePtsMap[x+y*width] = sub;
								}
							
							}
						}
					}
					
                        //heatmap.setFromPixels(eyePtsMap,width,height);
                    unsigned char *p = heatmap.getPixels();
                    for(int y = 0; y < height; y++)
                    {
                        memcpy(p, eyePtsMap + y*width, width*sizeof(unsigned char));
                        p += mov.width;
                    }
                    heatmap.flagImageChanged();
                    heatmap.scale(map_scalar, map_scalar);
				}
			}
		}
	}
	
	// else continue loading the eye-files and display the loading bar
	else
	{
		// if we have reached the last file in the directory
		if( eyeCounter == maxEyeListSize )
		{
			loadedFiles = true;

			// get rid of the loading bar
			// note this requires my hacked ofxgui
			panel3->active = false;
			panel1->active = true;
			panel2->active = true;
			panel6->active = true;
			
#ifndef _USEUNSTABLE
			panel5->active = true;
			panel4->active = true;
#endif

			//gui->removePanel(gui_Loader);
			gui->forceUpdate(true);

                //mov.play();
			if(isPaused)
			{	
				mov.setPaused(true);
			}
			mov.setLoopState(OF_LOOP_NONE);
			lastFrameNumber = -1;
			frameNumber = 1;
			// start to play the movie
            
            mov.setPosition(frameNumber / (float)numFrames);
                //			mov.setFrame(frameNumber);
		}
		else
		{
			// get the filename from the DIR structure
			string filename = myDIR.getPath(eyeCounter);
			// open it
			//printf("file #%d: %s\n", eyeCounter, filename.c_str());
			eyeList[eyeCounter].eyeFile.open(filename.c_str(), 
				ifstream::in);

			// get the subject prefix
			int indx1 = filename.rfind(string("/"));
			int indx2 = filename.find(string("."), indx1);
			if(indx2 > indx1)
				eyeList[eyeCounter].prefix = filename.substr(indx1+1,indx2-indx1-1);

			numFrames = 0;

			// while the file is not entirely read
			while(!eyeList[eyeCounter].eyeFile.eof())
			{
				// parse the values in the eye-tracking files,
				// and store in the current eyeList[] object
				string one, two, three, four, five, six, seven, eight, nine;
				float x,y,x2,y2,dil,dil2;
				int left_event,right_event;
				if(loadBinocular)
				{					
					// [frame] [left_x] [left_y] [left_dil] [left_event] [right_x] [right_y] [right_dil] [right_event]
					eyeList[eyeCounter].eyeFile >> frameCounter >> two >> three >> four >> five >> six >> seven >> eight >> nine;
					istringstream instr(two);
					instr >> x; eyeList[eyeCounter].eye_x.push_back(x);
					instr.clear();
					instr.str(three);
					instr >> y; eyeList[eyeCounter].eye_y.push_back(y);
					instr.clear();
					instr.str(four);
					instr >> dil; eyeList[eyeCounter].dilation.push_back(dil);
					instr.clear();
					instr.str(five);
					instr >> left_event; eyeList[eyeCounter].left_event.push_back(left_event);
					instr.clear();
					instr.str(six);
					instr >> x2; eyeList[eyeCounter].b_eye_x.push_back(x2);
					instr.clear();
					instr.str(seven);
					instr >> y2; eyeList[eyeCounter].b_eye_y.push_back(y2);
					instr.clear();
					instr.str(eight);
					instr >> dil2; eyeList[eyeCounter].b_dilation.push_back(dil2);
					instr.clear();
					instr.str(nine);
					instr >> right_event; eyeList[eyeCounter].right_event.push_back(right_event);

					// precompute the mean of the two eyes
					eyeList[eyeCounter].m_eye_x.push_back((x+x2)/2.);
					eyeList[eyeCounter].m_eye_y.push_back((y+y2)/2.);
					eyeList[eyeCounter].m_dilation.push_back((dil+dil2)/2.);

					eyeList[eyeCounter].is_fixation.push_back(right_event == 1 && left_event == 1);
				}
				else
				{
					// [frame] [x] [y] [dil]
					eyeList[eyeCounter].eyeFile >> frameCounter >> two >> three >> four;					
					istringstream instr(two);
					instr >> x; eyeList[eyeCounter].eye_x.push_back(x);
					instr.clear();
					instr.str(three);
					instr >> y; eyeList[eyeCounter].eye_y.push_back(y);
					instr.clear();
					instr.str(four);
					instr >> dil; eyeList[eyeCounter].dilation.push_back(dil);
				}
				numFrames++;
			}
			// close the file after reading it
			eyeList[eyeCounter].eyeFile.close();		
                //printf("Read %d frames for file %d/%d\n", numFrames, eyeCounter+1, maxEyeListSize);
			eyeCounter++;
			// the slider value is the percentage of files read
			float sliderVal = (float)eyeCounter/((float)eyeListSize);
			gui->update(gui_LoaderSlider, kofxGui_Set_Float, &sliderVal, sizeof(float));								
		}
	}


}


void diemDROI::updateEyesForCurrentFrame()
{
        //printf("Reading eye-movements for frame %d/%d\n", frameNumber, numFrames);
	int frame = MIN(frameNumber,numFrames);
	// reset eyeListSize to the max (because of blinks)
	eyeListSize = b_eyeListSize = m_eyeListSize = maxEyeListSize;
	if(!showMeanBinocular)
	{
		// get eye coords and store in xs and ys
		for (int i = 0, j = 0; i < eyeListSize; i++, j++)
		{
			// monocular data for the current frame
			xs[i] = eyeList[i].eye_x[frame]-offset_x;
			ys[i] = eyeList[i].eye_y[frame]-offset_y;

			// test for blinks ('--' is interpreted as a -41 or -7 in ASCII to int conversion)
			if(xs[i] > mov.width || xs[i] <= 0 ||
				ys[i] > mov.height || ys[i] <= 0 ||
				eyeList[i].left_event[frame] != 1) 
			{
				//printf("xs[%d]: %d ys[%d]: %d\n", i, xs[i], i, ys[i]);
				j--;
				eyeListSize--;
			}
			else
			{
				inputModelMap[2*j+0] = xs[i];
				inputModelMap[2*j+1] = ys[i];
				//printf("%d=%d\n%d=%d\n", 2*j+0, xs[i], 2*j+1, ys[i]);
			}
		}
		if(loadBinocular)
		{
			for (int i = 0, j = 0; i < b_eyeListSize; i++, j++)
			{
				// monocular data for the current frame
				b_xs[i] = eyeList[i].b_eye_x[frame]-offset_x;
				b_ys[i] = eyeList[i].b_eye_y[frame]-offset_y;	

				// test for blinks ('--' is interpreted as a -41 or -7 in ASCII to int conversion)
				if(b_xs[i] > mov.width || b_xs[i] <= 0 ||
					b_ys[i] > mov.height || b_ys[i] <= 0 ||
					eyeList[i].right_event[frame] != 1)
				{
					j--;
					b_eyeListSize--;
				}
				else
				{
					// start from 2*(eyeListSize) (the number of mono eye files 
					// without blinks) and add the bino data to the map
					inputModelMap[2*(eyeListSize) + 2*j+0] = b_xs[i];
					inputModelMap[2*(eyeListSize) + 2*j+1] = b_ys[i];
					//printf("%d=%d\n%d=%d\n", 2*(eyeListSize) + 2*j+0 , b_xs[i], 2*(eyeListSize) + 2*j+1, b_ys[i]);
				}
			}
		}
		//printf("%d, %d\n", eyeListSize, b_eyeListSize);
	}
	else
	{
		// get eye coords and store in xs and ys
		for (int i = 0, j = 0; i < m_eyeListSize; i++, j++)
		{
			// monocular data for the current frame
			xs[i] = eyeList[i].eye_x[frame]-offset_x;
			ys[i] = eyeList[i].eye_y[frame]-offset_y;	

			b_xs[i] = eyeList[i].b_eye_x[frame]-offset_x;
			b_ys[i] = eyeList[i].b_eye_y[frame]-offset_y;	

			m_xs[i] = (xs[i] + b_xs[i]) / 2;
			m_ys[i] = (ys[i] + b_ys[i]) / 2;

			if(frame-1 > 0)
			{
				// previous frame
				xs[i] = eyeList[i].eye_x[frame-1]-offset_x;
				ys[i] = eyeList[i].eye_y[frame-1]-offset_y;	

				b_xs[i] = eyeList[i].b_eye_x[frame-1]-offset_x;
				b_ys[i] = eyeList[i].b_eye_y[frame-1]-offset_y;	

				prev_m_xs[i] = (xs[i] + b_xs[i]) / 2;
				prev_m_ys[i] = (ys[i] + b_ys[i]) / 2;
				
				if(WITHIN(m_xs[i], prev_m_xs[i], 12) && WITHIN(m_ys[i], prev_m_ys[i], 12))
					scale_eyes[i] += .05f;
				else
					scale_eyes[i] = 1.0f;
			}
			


			// here we keep track of the mean eyes that fall in range and use this
			// for our inputMap for clustering (or eyePtsMap for gauss-dist), and for drawing the eyes
			// later on
			// test for blinks ('--' is interpreted as a -41 or -7 in ASCII to int conversion)
			if(m_xs[i] > mov.width || m_xs[i] <= 0 ||
				m_ys[i] > mov.height || m_ys[i] <= 0 ||
				eyeList[i].is_fixation[frame] != true)
			{
				//printf("xs[%d]: %d ys[%d]: %d\n", i, xs[i], i, ys[i]);
				j--;
				m_eyeListSize--;
			}
			else
			{
				inputModelMap[2*j+0] = m_xs[i];
				inputModelMap[2*j+1] = m_ys[i];
				//printf("%d=%d\n%d=%d\n", 2*j+0, m_xs[i], 2*j+1, m_ys[i]);
			}
		}
		eyeListSize = b_eyeListSize = m_eyeListSize;
		//printf("%d, %d, %d\n", m_eyeListSize,eyeListSize,b_eyeListSize);
	}
}


//#ifndef _USEUNSTABLE
void diemDROI::updateColorFlow()
{
#ifndef _USEUNSTABLE
	float scale = 0.5;
	int movwidth = mov.width * scale;
	int movheight = mov.height * scale;

	//// convert frames t and t+1 to grayscale float
	//// first get the current frame and the next frame:
	unsigned char * frame_t1 = mov.getPixels();

	motion_img.setFromPixels(frame_t1, mov.width, mov.height);
	
	ofxCvColorImage t1s;
	t1s.allocate(movwidth, movheight);
	t1s.scaleIntoMe(motion_img);
	
	ofxCvGrayscaleImage g1;
	g1.allocate(movwidth, movheight);
	
	//// convert to grayscale
	g1 = t1s;
	
	frame_t1 = g1.getPixels();
	//frame_t2 = g2.getPixels();

	//// convert to float
	float * frame_t1f = new float[movwidth*movheight];
	for(int x = 0; x < movwidth*movheight; x++) {
			frame_t1f[x]= (float)(frame_t1[x])/255.0f;
	}

	////vximg.setFromPixels(frame_t1f, bb_w, bb_h);
	////vyimg.setFromPixels(frame_t2f, bb_w, bb_h);	
	////cvNamedWindow("t1f", CV_WINDOW_AUTOSIZE);
	////cvShowImage( "t1f", vximg.getCvImage() );
	////cvNamedWindow("t2f", CV_WINDOW_AUTOSIZE);
	////cvShowImage( "t2f", vyimg.getCvImage() ); 	

	//// storage for flow
	float * colorflow = new float[movwidth*movheight*3];

	//// get the optical flow
	frameflowlib->getColorFlowField(frame_t1f, colorflow, movwidth, movheight);
	delete [] frame_t1f;
	//delete [] frame_t2f;

	//// convert to unsigned char
	unsigned char *pixels = new unsigned char[movwidth*movheight*3];	

	for(int x = 0; x < movwidth; x++) {
		for(int y = 0; y < movheight; y++) {
	//			// interleaved is coded as: 	3,w,h
	//			// planar is coded as:			w,h,3
				pixels[(y*movwidth+x)*3+0]= (unsigned char)(colorflow[(y*movwidth+x)+(movheight*movwidth*0)]*255.0f);
				pixels[(y*movwidth+x)*3+1]= (unsigned char)(colorflow[(y*movwidth+x)+(movheight*movwidth*1)]*255.0f);
				pixels[(y*movwidth+x)*3+2]= (unsigned char)(colorflow[(y*movwidth+x)+(movheight*movwidth*2)]*255.0f);
		}
	}
	delete [] colorflow;

	ofxCvColorImage cf_img;
	cf_img.allocate(movwidth, movheight);
	cf_img.setFromPixels(pixels, movwidth, movheight);

	//// scale back up to normal resolution
	motion_img.scaleIntoMe(cf_img);

	delete [] pixels;
#else
    motion_previous_previous_img = motion_previous_img;
    motion_previous_img = motion_this_img;
    
	ofxCvColorImage f;
	f.allocate(mov.getWidth(), mov.getHeight());
	f.setFromPixels(mov.getPixels(), mov.getWidth(), mov.getHeight());
    
	cvCvtColor(f.getCvImage(), motion_this_img.getCvImage(), CV_BGR2GRAY);
    
    // update optical flow
    opticalFlow->CalcFlow(motion_previous_previous_img.getCvImage(), motion_this_img.getCvImage(), 
                          motion_x_img.getCvImage(), motion_y_img.getCvImage(), bSaved);
    bSaved = 1;
    
    Mat fi = motion_dist_img.getCvImage();
    Mat fix = motion_x_img.getCvImage();
    Mat fiy = motion_y_img.getCvImage();
    Mat mi = motion_img.getCvImage();
    
    Mat fix2, fiy2;
    pow(fix, 2, fix2);
    pow(fiy, 2, fiy2);
    sqrt(fix2 + fiy2, fi);
    
    double maxval,minval;
    minMaxLoc(fi, &minval, &maxval);
    for(int r = 0; r < mi.rows; r++)
    {
        //unsigned char *rgb = (unsigned char *)cvMotionImg.getCvImage()->imageData + r*cvMotionImg.getCvImage()->widthStep;
        //float *val = (float *)cvFlowImg.getCvImage()->imageData + r*cvFlowImg.getCvImage()->widthStep;
        uchar *rgb = mi.ptr<uchar>(r);
        float *val = fi.ptr<float>(r);
        for(int c = 0; c < mi.cols; c++)
        {
            float fval = (float)*val / maxval * 255.0;
            jetColorMap(rgb, fval, 0, 255);
            rgb += 3;
            val++;
        }
    }

    motion_img.flagImageChanged();
#endif
}
//#else
//#endif

void diemDROI::updateFlicker()
{
	flicker_prev_img = flicker_this_img;

	ofxCvColorImage f;
	f.allocate(mov.getWidth(), mov.getHeight());
	f.setFromPixels(mov.getPixels(), mov.getWidth(), mov.getHeight());

	cvCvtColor(f.getCvImage(), flicker_this_img.getCvImage(), CV_BGR2GRAY);

	cvAbsDiff(flicker_this_img.getCvImage(), flicker_prev_img.getCvImage(), flicker_img.getCvImage());
	flicker_img.flagImageChanged();
}

void diemDROI::updateEdges()
{
	ofxCvColorImage f;
	f.allocate(mov.getWidth(), mov.getHeight());
	f.setFromPixels(mov.getPixels(), mov.getWidth(), mov.getHeight());

	cvCvtColor(f.getCvImage(), edge_img.getCvImage(), CV_BGR2GRAY);

	ofxCvFloatImage f2;
	f2.allocate(mov.getWidth(), mov.getHeight());
	cvSobel( edge_img.getCvImage(), f2.getCvImage(), 1, 1, 5);
    cvConvertScale(f2.getCvImage(), edge_img.getCvImage());
	edge_img.flagImageChanged();

}
//--------------------------------------------------------------
void diemDROI::draw(){
	
	if (!bSetup) {
		return;
	}
	
	ofBackground(0,0,0);
	
	ofSetHexColor(0xFFFFFF);
	
	int frames = frameNumber;//(int)(movie_time*(float)(numFrames-1.));

	if(loadedFiles && lastFrameNumber != -1)
	{
		if(true)
		{
			// set different alpha options,
			// in priority: alpha mask of a peekthrough set from:
				//	heatmap
				//	flicker
				//	edges
			if(showAlphaScreen)
			{
				glDisable(GL_BLEND);
				glColorMask(0, 0, 0, 1);
				glColor4f(1,1,1,1.0f);
				alphaScreen.loadData(heatmap.getPixels(),mov.width,mov.height,GL_ALPHA);
				alphaScreen.draw(0,0);
				glColorMask(1,1,1,0);
				glEnable(GL_BLEND);
				glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );
				
				if(drawOpticalFlow)
				{
					glColorMask(1,1,1,1);
					glColor4f(1,1,1,0.1f);
					drawColorFlow();
				}
				if(showEdges)
				{
					glColorMask(1,1,1,1);
					glColor4f(1,1,1,0.1f);
					drawEdges();
				}
				if(showFlicker)
				{
					glColorMask(1,1,1,1);
					glColor4f(1,1,1,0.1f);
					drawFlicker();
				}
					
				if(showMovie)
					mov.draw(0,0);
				
				glDisable(GL_BLEND);
			}
			else if(showFlicker)
			{
				// Draw the flicker into the alpha channel
				glDisable(GL_BLEND);
				glColorMask(0, 0, 0, 1);
				glColor4f(1,1,1,1.0f);
				alphaScreen.loadData(flicker_img.getPixels(),mov.width,mov.height,GL_ALPHA);
				alphaScreen.draw(0,0);
				glColorMask(1,1,1,0);
				glEnable(GL_BLEND);
				glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );

				// draw the flicker into the RGB channels
				glColorMask(1,1,1,1);
				glColor4f(1,1,1,0.1f);
				drawFlicker();

				if(drawOpticalFlow)
				{
					glColorMask(1,1,1,1);
					glColor4f(1,1,1,0.1f);
					drawColorFlow();
				}	
				if(showEdges)
				{
					glColorMask(1,1,1,1);
					glColor4f(1,1,1,0.1f);
					drawEdges();
				}
				if(showMovie)
					mov.draw(0,0);
				
				glDisable(GL_BLEND);
			}
			else
			{
				glDisable(GL_BLEND);
				
				if(showFlicker)
				{
					glEnable(GL_BLEND);
					glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );
					glColorMask(1,1,1,1);
					glColor4f(1,1,1,0.1f);
				}
				if(drawOpticalFlow)
				{
					glEnable(GL_BLEND);
					glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );
					glColorMask(1,1,1,1);
					glColor4f(1,1,1,0.1f);
					drawColorFlow();
					glDisable(GL_BLEND);
				}
				if(showEdges)
				{
					glEnable(GL_BLEND);
					glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );
					glColorMask(1,1,1,1);
					glColor4f(1,1,1,0.1f);
					drawEdges();
					glDisable(GL_BLEND);
				}
				if(showMovie)
				{
					glEnable(GL_BLEND);
					glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );
					glColorMask(1,1,1,1);
					glColor4f(1,1,1,0.2f);
					mov.draw(0,0);
					glDisable(GL_BLEND);
				}
/*
				float scale = 1;
				imageVectorizer iv(mov.getPixels(), mov.width, mov.height, 8, scale, 3);
				iv.segment();
				IplImage *myIplImage = cvCreateImage(cvSize(mov.width, mov.height), 8,3);
				iv.getResult(myIplImage);
				ofxCvColorImage myCvImage;
				//myCvImage.allocate(mov.width, mov.height);
				myCvImage.setFromIplImage(myIplImage, mov.width, mov.height);
				myCvImage.draw(0,0);
*/			
			}
		}

		
		if(showHistogram)
		{
			movCvImg.setFromPixels(mov.getPixels(),mov.width,mov.height);
			drawHistogram(movCvImg.getCvImage());
		}

		if(saveMovieImages)
		{
			saverImg.setFromPixels(mov.getPixels(), mov.width, mov.height, OF_IMAGE_COLOR);
			char fileOut[255];
			sprintf(fileOut, "imgs\\%s\\%s_%08d.png", movie_name.c_str(), movie_name.c_str(), frames);
			saverImg.saveImage(fileOut);
		}

		if(showHeatmap)
		{
			// create heatmap.  
			heatmap3.setFromPixels(colorized,mov.width,mov.height);
			heatmap3.blur(5);

			// scale to actual resolution
			heatmap3.scale(map_scalar,map_scalar);
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			ofSetColor(128,128,128,128);
			heatmap3.draw(0,0,mov.width,mov.height);
			glDisable(GL_BLEND);
		}
		
		
		if(showContours)
		{
			if(drawOpticalFlow)
				ofSetColor(0,0,0);
			else
				ofSetColor(255,255,255);
			for(int j = 50; j < 255; j+=50)
			{
				heatmap.threshold(j, false);
				me.findContours(heatmap, 100, (mov.width*mov.height)/2, 100, false, false);				
				me.draw(0,0);	
                heatmap.swapTemp();
			}
		}

		drawEyes();

		ofSetHexColor(0xFFFFFF);
		//ofRect(newPatch.left, newPatch.upper, newPatch.width, newPatch.height);

		ofSetHexColor(0xFFFFFF);
		string tagString = "Copyright 2011 (CC-SA-NC) The DIEM Project (http://thediemproject.wordpress.com)";
		ofDrawBitmapString(tagString.c_str(), 25, mov.height-25);
		
		// draw the movie time as text above the movie time slider
		ofSetHexColor(0xFFFFFF);
		char reportString[255];
		int minutes = (int)(mov.getPosition()*mov.getDuration())%3600/60;
            //int minutes = ((int)(frameNumber / FPS) % 3600) / 60.;
		int seconds = (int)(mov.getPosition()*mov.getDuration())%60;
            //int seconds = (int)(frameNumber / FPS) % 60;
		if ( movhr > 0 )	{
			int hours = mov.getPosition()*mov.getDuration()/3600.;
                //int hours = frameNumber / FPS / 60. / 60.;
			sprintf(reportString, "%02d:%02d.%02d/%02d:%02d.%02d", hours, minutes, seconds, movhr, movmin, movsec);
		}
		else	{
			sprintf(reportString, "%02d:%02d/%02d:%02d", minutes, seconds, movmin, movsec);
		}
		ofDrawBitmapString(reportString,12,mov.height+12);

		// draw the current frame number
		sprintf(reportString, "Frame: %d", frames);
		ofDrawBitmapString(reportString, mov.width-(80 + ((int)log10((float)frames)*10)), mov.height+12);

		if(showRecording && !isPaused && !doneRecording)
		{				
			//if(mov.isFrameNew())
			//{
				// this should be pre-computed as the highest power of 
				// 32bit multiple greater than the mov size
				// see http://download.nvidia.com/developer/Papers/2005/Fast_Texture_Transfers/Fast_Texture_Transfers.pdf 

				// Asynchronous readback
				glReadBuffer(GL_BACK);

				glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[0]);
				glReadPixels(0, 0, screen_width, screen_height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
				//glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, 0);
				glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

				//glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[1]);
				//glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_BGRA, GL_UNSIGNED_BYTE, 0);
				//glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

				glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[0]);
				GLubyte* src = (GLubyte*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);
            
            int offset = screen_height - mov.height;
            printf("(%d,%d), (%d,%d)\n", mov.width, mov.height, ofGetScreenWidth(), ofGetScreenHeight());

				// save movie // RGB <- BGRA
				if(src)
				{	
					for (int x = 0; x < mov.width; x++)	
					{
						for (int y = 0; y < mov.height; y++)	
						{
							colorized[(x+y*(mov.width))*3+0] = src[(x+(mov.height-y-1 +offset)*(mov.width))*4+2];		
							colorized[(x+y*(mov.width))*3+1] = src[(x+(mov.height-y-1 +offset)*(mov.width))*4+1];				
							colorized[(x+y*(mov.width))*3+2] = src[(x+(mov.height-y-1 +offset)*(mov.width))*4+0];						
						}
					}		
					//memcpy(colorized, src, sizeof(unsigned char)*mov.width*mov.height*3);
					saver.addFrame(colorized, 1.0f / FPS); 
				}
				
				//saver.addFrame(src, 1.0f / FPS);
				
				glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);     // release pointer to the mapped buffer	
				glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);		
				
				
			//}
		}

	}	
	else	{
		if(eyeCounter < eyeListSize)
		{
			string file = myDIR.getPath(eyeCounter);
			ofDrawBitmapString(file,20,mov.height/2-5);
			char text[256];
			sprintf(text, "Loading %d files", eyeListSize);
			ofDrawBitmapString(text,20,mov.height/2-20);
		}
	}

	// draw the movie timeline and time information
	if(loadedFiles)
	{
		drawMovieControls();
	}

#ifndef _USEUNSTABLE
	// draw all drois
	for(vector<pkmDROI *>::iterator itr=objs.begin(); itr!=objs.end(); ++itr)
	{
		(*itr)->draw();
	}
#endif

	// draw the user interface
	gui->draw();

	// draws x,y coordinates at cursor
	if( isPaused && mouseX < mov.width && mouseX > 0 && mouseY < mov.height && mouseY > 0 )
	{
		char buf[256];
		sprintf(buf, "%d, %d", mouseX, mouseY);
		ofDrawBitmapString(buf, (float) mouseX, (float) mouseY);		
	}
}


void diemDROI::drawEyes()
{
					
	// draw eyes
	if(showMeanBinocular)
	{
		/*if(showAllFixations && isPaused)
		{
			for(int f = 1; f < numFrames; f++)
			{
				// draw this frames eyes
				for (int i = 0; i < m_eyeListSize; i++)
				{
					xs[i] = eyeList[i].eye_x[f-1]-offset_x;
					ys[i] = eyeList[i].eye_y[f-1]-offset_y;	

					b_xs[i] = eyeList[i].b_eye_x[f-1]-offset_x;
					b_ys[i] = eyeList[i].b_eye_y[f-1]-offset_y;	

					m_xs[i] = (xs[i] + b_xs[i]) / 2;
					m_ys[i] = (ys[i] + b_ys[i]) / 2;

					if(showEyes && m_xs[i] > 0 && m_ys[i] > 0)
					{
#ifndef _USEUNSTABLE
						// display an ellipse at the x,y coordinates (minus offset)
						if(drawOpticalFlow)
							ofSetColor(0, 0, 0);
						else
#endif
							ofSetColor(255, 255, 255);			
						ofEllipse(m_xs[i], m_ys[i], 13, 8);

						int dil = eyeList[i].m_dilation[f-1];
						// dilated pupils
						ofSetColor(0, 255, 0);		
						ofCircle(m_xs[i], m_ys[i], dil/1200.+4.);
					}

					if(showSaccades && f > 1)
					{
						xs[i] = eyeList[i].eye_x[f-2]-offset_x;
						ys[i] = eyeList[i].eye_y[f-2]-offset_y;	

						b_xs[i] = eyeList[i].b_eye_x[f-2]-offset_x;
						b_ys[i] = eyeList[i].b_eye_y[f-2]-offset_y;	

						int xsp = (xs[i] + b_xs[i]) / 2;
						int ysp = (ys[i] + b_ys[i]) / 2;

						if(xsp > 0 && ysp > 0 && xsp < mov.width && ysp < mov.height)
						{
							if(showEyes)
							{
#ifndef _USEUNSTABLE
								// display an ellipse at the x,y coordinates (minus offset)
								if(drawOpticalFlow)
									ofSetColor(0, 0, 0, 100);
								else
#endif
									ofSetColor(255, 255, 255, 100);					
								ofEllipse(xsp, ysp, 13, 8);

								int dil = eyeList[i].m_dilation[f-2];
								// dilated peoples
								ofSetColor(0, 255, 0, 100);		
								ofCircle(xsp, ysp, dil/1200.+4.);
							}
						
							if(m_xs[i] > 0 && m_ys[i] > 0 && m_xs[i] < mov.width && m_ys[i] < mov.height)
								ofLine(m_xs[i], m_ys[i], xsp, ysp);
						}
					}
				}

			}
		}

		else
		{*/
			// only draw this frames eyes
			for (int i = 0; i < m_eyeListSize; i++)
			{
				if(m_xs[i] > 0 && m_ys[i] > 0 && m_xs[i] < mov.width && m_ys[i] < mov.height)
				{
					if(prev_m_xs[i] > 0 && prev_m_ys[i] > 0 && prev_m_xs[i] < mov.width && prev_m_ys[i] < mov.height)
					{
						if(showSaccades)
							ofLine(m_xs[i], m_ys[i], prev_m_xs[i], prev_m_ys[i]);
					}
					else
						scale_eyes[i] = 1.0f;

					// show eyes getting bigger
					if(showEyes)
					{
#ifndef _USEUNSTABLE
						if(drawOpticalFlow)
							ofSetColor(0, 0, 0);
						else
#endif
						ofSetColor(255, 255, 255);			
						ofEllipse(m_xs[i], m_ys[i], scale_eyes[i]*11, scale_eyes[i]*8);

						int dil = eyeList[i].m_dilation[frameNumber];
						// dilated pupils
						ofSetColor(0, 255, 0);		
						ofCircle(m_xs[i], m_ys[i], scale_eyes[i]*dil/1200.+4.);
					}
					if(showSubjectNames)
					{
						ofDrawBitmapString(eyeList[i].prefix, m_xs[i], m_ys[i]);	
						char buf[256];
						sprintf(buf, "%d/%d subjects", m_eyeListSize, maxEyeListSize);
						ofSetColor(255, 255, 255);		
						ofDrawBitmapString(buf, mov.width/2, mov.height+15);
					}
				}
			}
		//}
	}
	else
	{
		for (int i = 0; i < eyeListSize; i++)
		{
			// display an ellipse at the x,y coordinates (minus offset)
			ofSetColor(255, 255, 255);			
			int dil = eyeList[i].dilation[frameNumber];
			ofEllipse(xs[i], ys[i], 13, 8);
			// dilated pupils
			//ofSetColor(0, 0, 255);		
			ofSetColor(255, 0, 0);		
			ofCircle(xs[i], ys[i], dil/1200.+4.);
			
				//char out2[80];
				//sprintf(out2, "%d, %d", xs[i], ys[i]);
				//ofDrawBitmapString(out2, xs[i]+2, ys[i]+2);
			//ofDrawBitmapString("L", xs[i]+2, ys[i]+2);
		}
		if(loadBinocular)
		{
			for (int i = 0; i < b_eyeListSize; i++)
			{
				// display an ellipse at the x,y coordinates (minus offset)
				ofSetColor(255, 255, 255);			
				int dil = eyeList[i].b_dilation[frameNumber];
				ofEllipse(b_xs[i], b_ys[i], 13, 8);
				// dilated pupils
				ofSetColor(255, 0, 0);		
				ofCircle(b_xs[i], b_ys[i], dil/1200.+4.);
				//char out[80];
				//sprintf(out, "%d, %d", b_xs[i], b_ys[i]);
				//ofDrawBitmapString(out, b_xs[i]+2, b_ys[i]+2);
			}

			// draw lines between the eyes... we could use the xs and b_xs array, but there is a chance
			// that one of the eyes will have gone off screen or blinked and then the index will not
			// match between the two
			/*ofSetColor(0,255,0);
			for(int i = 0; i < MIN(eyeListSize,b_eyeListSize); i++)
			{
				
				ofLine(eyeList[i].b_eye_x[movie_time*(numFrames-1)] - offset_x, eyeList[i].b_eye_y[movie_time*(numFrames-1)] - offset_y, 
					eyeList[i].eye_x[movie_time*(numFrames-1)] - offset_x, eyeList[i].eye_y[movie_time*(numFrames-1)] - offset_y);
				//ofLine(xs[i], ys[i], b_xs[i], b_ys[i]);
			}*/
		}
	}
}

//#ifndef _USEUNSTABLE
void diemDROI::drawColorFlow()
{
	//// draw the optical flow
	motion_img.draw(0,0);
	//mov.nextFrame();
}
//#else
//#endif

void diemDROI::drawEdges()
{
	edge_img.draw(0,0);
}

void diemDROI::drawFlicker()
{
	flicker_img.draw(0,0);
}

void diemDROI::drawMovieControls()
{

	if(isPaused)
	{
		glEnable(GL_BLEND);
		ofSetColor(255, 255, 255, 100);
		ofFill();
		
		// alpha pause button over the movie
		ofRect(mov.width/2. - 35, mov.height * 2.2 / 5.,30, mov.height / 5.);
		ofRect(mov.width/2. + 35, mov.height * 2.2  / 5.,30, mov.height / 5.);

		// pause button control
		ofRect(6, mov.height+15, 4, 12);
		ofRect(12, mov.height+15, 4, 12);
		glDisable(GL_BLEND);


	}
	else
	{
		ofSetColor(0, 255, 0);
		ofFill();

		// draw the play button
		ofSetPolyMode(OF_POLY_WINDING_ODD);	// this is the normal mode
		ofBeginShape();
		ofVertex(6,mov.height+15);
		ofVertex(16,mov.height+21);
		ofVertex(6,mov.height+27);
		ofEndShape();
	}
}

//--------------------------------------------------------------
void diemDROI::keyPressed  (int key){ 

	if(key == 112)						// 'p' to enable gui
	{
		//gui->activate(!gui->mIsActive);
		panel1->active = !panel1->active;
		panel6->active = !panel6->active;	

#ifndef _USEUNSTABLE
		panel5->active = !panel5->active;
		panel4->active = !panel4->active;
#endif

	}
	else if(key == 'z' || key == OF_KEY_LEFT)
	{
		updatedFrame = true;
		frameNumber = MAX(frameNumber-1,1);
            //		mov.setFrame(frameNumber);
        
        mov.setPosition(frameNumber / (float)numFrames);
		// update the movie slider to reflect the current movie time
		//gui->update(gui_MovieTimeSlider, kofxGui_Set_Float, 
		//	&movie_time, sizeof(float));
	}
	else if(key == 'c' || key == OF_KEY_RIGHT)
	{
		updatedFrame = true;
		frameNumber = MIN(frameNumber+1,numFrames);
            //		mov.setFrame(frameNumber);
        
        mov.setPosition(frameNumber / (float)numFrames);
		// update the movie slider to reflect the current movie time
		//gui->update(gui_MovieTimeSlider, kofxGui_Set_Float, 
		//	&movie_time, sizeof(float));
	}
	else if(key == 'x' || key == OF_KEY_UP || key == ' ')
	{
		isPaused = !isPaused;
		if(showRealTime)
			mov.setPaused(isPaused);
	}
	else if(key == 'q')
	{
		movieSpeed -= 0.05f;
		mov.setSpeed(movieSpeed);
		mov.setPaused(isPaused);
	}
	else if(key == 'w')
	{
		movieSpeed = 1.0f;
		mov.setSpeed(movieSpeed);
		mov.setPaused(isPaused);
	}
	else if(key = 'e')
	{
		movieSpeed += 0.05f;
		mov.setSpeed(movieSpeed);
		mov.setPaused(isPaused);
	}
}

//--------------------------------------------------------------
void diemDROI::keyReleased(int key)
{ 

}

//--------------------------------------------------------------
void diemDROI::mouseMoved(int x, int y )
{
	
}

//--------------------------------------------------------------
void diemDROI::mouseDragged(int x, int y, int button)
{
	gui->mouseDragged(x, y, button);	
}

//--------------------------------------------------------------
void diemDROI::mousePressed(int x, int y, int button)
{
	//if( current_obj < 0  || !objs[current_obj]->isMouseOver() )
	//{
	//	if( !(x < mov.width/3) && y < mov.height )
	//	{
	//		isPaused = !isPaused;
	//		mov.setPaused(isPaused);
	//		//	set flag to draw paused thing
	//	}
	//	gui->mousePressed(x, y, button);
	//}

	//if(current_obj < 0 || !objs[current_obj]->isMouseOver())
		gui->mousePressed(x, y, button);
	//if(current_obj >= 0 && objs[current_obj]->isMouseOver() && tracker_type == boosting && tracker != NULL)
	//{
	//	int _x, _y, _x2, _y2;
	//	objs[current_obj]->getBounds(_x, _y, _x2, _y2);
	//	int w = _x2 - _x + 1;
	//	int h = _y2 - _y + 1;
	//	boosting::Rect oldPatch = tracker->getTrackedPatch();
	//	boosting::Rect newPatch(_x, _y, oldPatch.height, oldPatch.width);
	//	tracker->manualUpdate(newPatch);
	//}
	mouse_state_down = true;
}

//--------------------------------------------------------------
void diemDROI::mouseReleased()
{
	gui->mouseReleased(mouseX, mouseY, 0);
	mouse_state_down = false;
}

void diemDROI::handleGui(int parameterId, int task, void* data, int length)
{
	reset = true;
	bool f = false, t = true;
	switch(parameterId)
	{
	case gui_CovarianceSwitch:
		cov_type = *(int *)data;
		break;
		case gui_ScalarsSwitch:
		if(task == kofxGui_Set_Int)
		{	
			int i = *(int *)data;
			switch(i)
			{
			case 0:
				map_scalar = 1;
				break;
			case 1:
				map_scalar = 2;
				break;
			case 2:
				map_scalar = 4;
				break;
			case 3:
				map_scalar = 6;
				break;
			case 4:
				map_scalar = 8;
				break;
			}

		}
		break;
	case gui_ResSwitch:
		if(task == kofxGui_Set_Int)
		{	
			int i = *(int *)data;
			switch(i)
			{
			case 0:
				offset_x = (640 - mov.width) / 2;
				offset_y = (480 - mov.height) / 2;
				break;
			case 1:
				offset_x = (800 - mov.width) / 2;
				offset_y = (600 - mov.height) / 2;
				break;
			case 2:
				offset_x = (1024 - mov.width) / 2;
				offset_y = (768 - mov.height) / 2;
				break;
			case 3:
				offset_x = (1280 - mov.width) / 2;
				offset_y = (960 - mov.height) / 2;
				break;
			case 4:
				offset_x = (1440 - mov.width) / 2;
				offset_y = (900 - mov.height) / 2;
				break;
			case 5:
				offset_x = (1600 - mov.width) / 2;
				offset_y = (1200 - mov.height) / 2;
				break;
			}
		}			
		break;
	case gui_ClustersSwitch:
		{
			int i = *(int *)data;
			switch(i)
			{
			case 0:
				minClusterComponents = 1; maxClusterComponents = 1; break;
			case 1:
				minClusterComponents = 2; maxClusterComponents = 2; break;
			case 2:
				minClusterComponents = 3; maxClusterComponents = 3; break;
			case 3: 
				minClusterComponents = 4; maxClusterComponents = 4; break;
			case 4:
				minClusterComponents = 8; maxClusterComponents = 8; break;
			case 5:
				minClusterComponents = 1; maxClusterComponents = 2; break;
			case 6:
				minClusterComponents = 1; maxClusterComponents = 4; break;
			case 7:
				minClusterComponents = 1; maxClusterComponents = 8; break;
			case 8:
				minClusterComponents = 2; maxClusterComponents = 4; break;
			case 9:
				minClusterComponents = 4; maxClusterComponents = 8; break;	
			}
			break;
		}
	case gui_EyefilesSwitch:
		maxEyeListSize = *(float *)data;
		break;
	case gui_MovieTimeSlider:
		if(task == kofxGui_Set_Float)
		{
			movie_time = *(float *)data;
                //			frameNumber = movie_time*mov.getDuration()*FPS + 1;
            frameNumber = movie_time * numFrames;
			//if(frameNumber > numFrames)
			//{
				frameNumber = MIN(numFrames,frameNumber);
                //				mov.setFrame(frameNumber);
            mov.setPosition(movie_time);
			//}
			//else
			//{
			//	mov.setPosition(movie_time);
			//}
		}
		break;

	case gui_EnableEyes:
		showEyes = !showEyes;
		break;
	case gui_EnableSaccades:
		showSaccades = !showSaccades;
		break;
	case gui_EnableSubjectNames:
		showSubjectNames = !showSubjectNames;
		break;
#ifndef _USEUNSTABLE
	case gui_EnableAllFixations:
		showAllFixations = !showAllFixations;
		break;
#endif
	case gui_EnableMeanBinocular:
		showMeanBinocular = !showMeanBinocular;
		break;
	case gui_EnableHistogram:
		showHistogram = !showHistogram;
		break;
	case gui_EnableMovie:
		showMovie = !showMovie;
		break;
	case gui_EnableRecording:
		showRecording = !showRecording;
		if(!saver.bAmSetupForRecording())
			initializeMovieOutput();
		break;
	case gui_EnableSaveMovieImages:
		saveMovieImages = !saveMovieImages;
		break;
	case gui_EnableNormalized:
		showNormalized = !showNormalized;
		break;
	case gui_EnableRealTime:
		showRealTime = !showRealTime;
		break;
	case gui_EnableClustering:
		showClustering = !showClustering;
		break;
	case gui_EnableHeatmap:
		showHeatmap = !showHeatmap;
		break;
	case gui_EnableContours:
		showContours = !showContours;
		break;
	case gui_EnableAlphaScreen:
		showAlphaScreen = !showAlphaScreen;
		break;

#ifndef _USEUNSTABLE
	case gui_TrackCAMShift:
		panel4->update(gui_TrackCAMShift, 0, (void *) &t, sizeof(bool));
		panel4->update(gui_TrackFlow, 0, (void *) &f, sizeof(bool));
		panel4->update(gui_TrackBoosting, 0, (void *) &f, sizeof(bool));
		tracker_type = cam_shift;
		break;
	case gui_TrackFlow:
		panel4->update(gui_TrackCAMShift, 0, (void *) &f, sizeof(bool));
		panel4->update(gui_TrackFlow, 0, (void *) &t, sizeof(bool));
		panel4->update(gui_TrackBoosting, 0, (void *) &f, sizeof(bool));
		tracker_type = dense_optical_flow;
		break;
	case gui_TrackBoosting:
		panel4->update(gui_TrackCAMShift, 0, (void *) &f, sizeof(bool));
		panel4->update(gui_TrackFlow, 0, (void *) &f, sizeof(bool));
		panel4->update(gui_TrackBoosting, 0, (void *) &t, sizeof(bool));
		tracker_type = boosting;
		break;
	case gui_ImportFromIAS:
		if(mouse_state_down)
		{
			importFromIAS();
		}
		break;
	case gui_ExportAllObjects:
		if(mouse_state_down)
		{
			int object_num = 1;
			for( vector<pkmDROI *>::iterator it = objs.begin();
				it != objs.end(); ++it )
			{
				string filename = "data\\ias\\" + movie_name + "\\" + (*it)->my_name + ".ias";
				(*it)->outputToIAS(filename, movie_name, offset_x, offset_y, object_num++);
			}
		}
		break;
	case gui_NewFHObjectTrigger:
	case gui_NewRectObjectTrigger:
		if(mouse_state_down && !loading_obj)
		{
			printf("creating new object...");
			// check for loading object so the user doesn't press the new object
			// button while still making vertices for the current object!
			loading_obj = true;
			// add a new object to the ID list
			int idx = total_gui_elements + gui_Objects.size() + 1;
			printf("current object: %d\n", current_obj);

			char current_object_str[100] = {0};
			INT_PTR ok = CWin32InputBox::InputBox("Object Name", "", current_object_str, 100, false);
			
			if(ok != IDOK)
				return;
			// give it a name
			//char current_object_str[256];
			//sprintf(current_object_str, "object_%d", gui_Objects.size());

			
			printf("adding gui element...");
			// add the object gui
			ofxGuiFiles *obj = (ofxGuiFiles *)panel5->addFiles(idx, "", 
				string(current_object_str).length()*7.5, OFXGUI_FILES_HEIGHT, current_object_str, "", "");
			obj->mState = 2;
			gui_Objects.push_back(obj);

			//// unselect all the other objects from record (make them play instead)
			//for(int i = 0; i < gui_Objects.size() - 1; i++)
			//{
			//	if(gui_Objects[i]->mState == 2)
			//		gui_Objects[i]->mState = 1;
			//}

			printf("creating droi...");
			pkmDROI *o = new pkmDROI();
			if(parameterId == gui_NewFHObjectTrigger)
				o->setup(0, 0, mov.width, mov.height, numFrames, 1, string(current_object_str));
			else
				o->setup(0, 0, mov.width, mov.height, numFrames, 2, string(current_object_str));
			o->setRecordState(2);
			objs.push_back(o);
			current_obj = gui_Objects.size() - 1;

			// add a new tracker in the user wants to track with boosting
			//pkmDROITracker *t = NULL;
			printf("adding tracker...");
			pkmBoostingTracker *t = NULL;
			trackers.push_back(t);
			loading_obj = false;
			printf("done loading\n");
		}
		break;
	case gui_TrackObjectBtn:
		// user wants to track the current object
		if( current_obj >=0 && trackButton->mValue )
		{
			trackCurrentObject = true;
		}
		else
		{
			trackCurrentObject = false;
		}
		break;
	case gui_TrackAdaptiveBtn:
		// setup adaptive tracking
		useAdaptTrack = !useAdaptTrack;
		if(current_obj >= 0 && trackers[current_obj] != NULL)
		{
			//trackers[current_obj]->useAdaptiveTracking(useAdaptTrack);
			// delete the current tracker so it is reinitialized with adaptive
			// tracking on the next update
			delete trackers[current_obj];
			trackers[current_obj] = NULL;
		}
		break;
	case gui_BinDROIs:
		binDROIs = !binDROIs;
		break;
	case gui_InitDROIs:
		for(vector<pkmDROI *>::iterator itr=objs.begin(); itr!=objs.end(); ++itr)
		{
			(*itr)->reinitialize(0, 0, mov.width, mov.height);
		}
		break;
#endif
    case gui_DrawOpticalFlowBtn:
        drawOpticalFlow = !drawOpticalFlow;
        break;
	case gui_DrawFlicker:
		showFlicker = !showFlicker;
		break;
	case gui_DrawEdges:
		showEdges = !showEdges;
		break;
	}

#ifndef _USEUNSTABLE
	// dynamic cases:
	// the user has selected a new object
	if( parameterId > total_gui_elements )
	{
		current_obj = parameterId - total_gui_elements - 1;
		// set the droi to the state of the gui thing
		objs[current_obj]->setRecordState(gui_Objects[current_obj]->mState);		

		printf("selected: %d\n", parameterId - total_gui_elements - 1);
		// for all the parameters that are dynamically created (i.e. > total_gui_elements)
		// this is to ensure that if we select another object for record
		// we turn off any other objects record state (one record at a time?)
		//if(gui_Objects[current_obj]->mState == 2)
		//{
		//	int num_objects = gui_Objects.size();
		//	for(int i = 0; i < num_objects && parameterId > total_gui_elements; i++)
		//	{
		//		int idx = i+total_gui_elements+1;
		//		if(parameterId != idx && gui_Objects[i]->mState == 2)
		//		{
		//			gui_Objects[i]->mState = 1;
		//			objs[i]->setRecordState(1);
		//		}
		//	}
		//}
	}
#endif
}


#ifndef _USEUNSTABLE
void diemDROI::trackObject()
{
	if(tracker_type == dense_optical_flow)
	{

		// go through all the objects that are set to record state
		int i = 0;
		for(vector<pkmDROI *>::iterator itr=objs.begin(); itr!=objs.end(); ++itr, i++)
		{
			if((*itr)->state == 2)
			{
				// get the bounding box of the object around a radius
				int x, y, x2, y2;
				int radius = 0;
				(*itr)->getBounds(x, y, x2, y2);
				// check it is within movie bounds
				x = MAX(0, x-radius);
				x2 = MIN(x2+radius, mov.width-1);
				y = MAX(0, y-radius);
				y2 = MIN(y2+radius, mov.height-1);

				int bb_w = x2-x+1;
				int bb_h = y2-y+1;

				ofxCvFloatImage vximg, vyimg;
				vximg.allocate(bb_w, bb_h);
				vyimg.allocate(bb_w, bb_h);
				//vximg.allocate(mov.width, mov.height);
				//vyimg.allocate(mov.width, mov.height);

				ofxCvColorImage t1;
				t1.allocate(mov.width, mov.height);
				unsigned char * frame_t1 = mov.getPixels();
				t1.setFromPixels(frame_t1, mov.width, mov.height);
				t1.setROI(x, y, bb_w, bb_h);
				
				ofxCvGrayscaleImage g1;
				g1.allocate(mov.width, mov.height);
				g1.setROI(x, y, bb_w, bb_h);
				
				// convert to grayscale
				g1 = t1;
				
				frame_t1 = g1.getPixels();
				
				float * frame_t1f = new float[bb_w*bb_h];
				//float * frame_t1f = new float[mov.width*mov.height];
				int idx, idy;
				for(int sub = 0; sub < bb_w*bb_h; sub++) {
				//for(int sub = 0; sub < mov.width*mov.height; sub++) {
					frame_t1f[sub]= (float)(frame_t1[sub])/255.0f;
				}
				
				vximg.setFromPixels(frame_t1f, bb_w, bb_h);
				//cvNamedWindow("t1f", CV_WINDOW_AUTOSIZE);
				//cvShowImage( "t1f", vximg.getCvImage() );
				//cvWaitKey(0);
							
				// storage for flow
				float * vx = new float[bb_w*bb_h];
				float * vy = new float[bb_w*bb_h];
				//float * vx = new float[mov.width*mov.height];
				//float * vy = new float[mov.width*mov.height];

				// get the optical flow
				if(myflowlib->getFlow(frame_t1f, vx, vy, bb_w, bb_h))
				//if(myflowlib->getFlow(frame_t1f, vx, vy, mov.width, mov.height))
				{
					// put into a ofxcv container
					vximg.setFromPixels(vx, bb_w, bb_h);
					vyimg.setFromPixels(vy, bb_w, bb_h);	
					//vximg.setFromPixels(vx, mov.width, mov.height);	
					//vyimg.setFromPixels(vy, mov.width, mov.height);	

					//vximg.setROI(x, y, bb_w, bb_h);
					//vyimg.setROI(x, y, bb_w, bb_h);

					//cvNamedWindow("X", CV_WINDOW_AUTOSIZE);
					//cvShowImage( "X", vximg.getCvImage() ); 
					//cvNamedWindow("Y", CV_WINDOW_AUTOSIZE);
					//cvShowImage( "Y", vyimg.getCvImage() );
					
					//double deltax, deltay, crap; 
					//CvPoint minpt, maxpt;

					//cvMinMaxLoc(vximg.getCvImage(), &crap, &deltax, &minpt, &maxpt);
					//cvMinMaxLoc(vyimg.getCvImage(), &crap, &deltay, &minpt, &maxpt);
					
					CvScalar deltax = cvAvg(vximg.getCvImage());
					CvScalar deltay = cvAvg(vyimg.getCvImage());

					(*itr)->setNextFrame();
					(*itr)->moveMe(deltax.val[0], deltay.val[0]);

					//(*itr)->setNextFrame();
					//(*itr)->moveMe(deltax, deltay);

					//// draw the flow with pretty colors
					float *colorFlowPixels = new float[bb_w*bb_h*3];
					myflowlib->getColorFlowField(colorFlowPixels, bb_w, bb_h);

					ofxCvColorImage colorFlow;
					colorFlow.allocate(bb_w, bb_h);
					for(int sub = 0; sub < bb_w*bb_h; sub++)
					{
						for(int dim = 0; dim < 3; dim++)
							colorFlow.getCvImage()->imageData[sub*3+(2-dim)] = (char)(colorFlowPixels[sub+(bb_w*bb_h*dim)]*255.0f);
					}

					cvNamedWindow("Motion", CV_WINDOW_AUTOSIZE);
					cvShowImage( "Motion", colorFlow.getCvImage() );

					//delete [] colorFlowPixels;

				}
				delete [] frame_t1f;
				delete [] vx;
				delete [] vy;
			}
		}
	
	}
	else {	// 	boosting

		// get the movie as grayscale
		ofxCvColorImage t1;
		t1.allocate(mov.width, mov.height);
		t1.setFromPixels(mov.getPixels(), mov.width, mov.height);
		
		ofxCvGrayscaleImage g1;
		g1.allocate(mov.width, mov.height);
		
		// convert to grayscale
		g1 = t1;
		
		unsigned char *pixels = g1.getPixels();

		// go through all the objects that are set to record state
		int i = 0;
		for(vector<pkmDROI *>::iterator itr=objs.begin(); itr!=objs.end(); ++itr, i++)
		{
			if((*itr)->state == 2)
			{
				// if the user has manually updated the location because of crap tracking, 
				// then we have to reset the tracker.
				if((*itr)->wasUpdatedByMouse() && trackers[i] != NULL)
				{
					printf("deleting object %d's tracker...", current_obj);
					(*itr)->clearUpdatedByMouseFlag();
					delete trackers[i];
					trackers[i] = NULL;
					printf("done\n");
				}

				if(trackers[i] == NULL)
				{
					printf("initializing object %d's tracker...", current_obj);
					// get the bounding box of the object around a radius
					int x, y, x2, y2;
					int radius = 0;
					printf("getting object bounds...");
					(*itr)->getBounds(x, y, x2, y2);

					// check it is within movie bounds
					x = MAX(0, x-radius);
					x2 = MIN(x2+radius, mov.width-1);
					y = MAX(0, y-radius);
					y2 = MIN(y2+radius, mov.height-1);

					int bb_w = x2-x;
					int bb_h = y2-y;

					// initialize the tracker
					printf("creating tracker...");
					trackers[i] = new pkmBoostingTracker(pixels, 
						mov.width-1, mov.height-1, 
						x, y, bb_w, bb_h,
						useAdaptTrack);
					//vector<CvPoint> * pts = objs[current_obj]->getVertices();
					//trackers[current_obj] = new pkmDROITracker(pixels, mov.width, mov.height, x, y, bb_w, bb_h, pts);
					printf("done\n");
				
				}
				else
				{
					printf("tracking frame %d...", frameNumber);
					trackers[i]->trackNextFrame(pixels);
					printf("done\n");
					int x, y, w, h;
					//vector<CvPoint> pts;
					//trackers[current_obj]->getCurrentVertices(&pts);
					printf("getting ROI\n");
					trackers[i]->getCurrentROI(x, y, w, h);
					(*itr)->setNextFrame();
					//objs[current_obj]->setVertices(pts);
					printf("moving\n");
					(*itr)->moveMeTo(x, y, w, h);

					//IplImage *mask = cvCreateImage(cvSize(mov.width,mov.height), IPL_DEPTH_8U, 1);
					//objs[current_obj]->getConvexPoly(mask);
					//double * seg = segmenter.segment(mov.getPixels(), mov.width, mov.height, (unsigned char *)mask->imageData);

				} // end if/else
			} // end if
		} // end for

	}
}
#else
#endif

inline float diemDROI::gaussDist(int &x, int mean)
{
	return (A*expf(B*(x-mean)*(x-mean)));
	//return (1./sqrt(TWO_PI*sigma*sigma))*exp(-(1/(sigma*sigma))*(x-mean)*(x-mean));
}

inline float diemDROI::gaussDist(float x, float y, float meanx, float meany, float sigmax, float sigmay)
{
	return (1.0f/sqrtf(TWO_PI*sigmax*sigmay))*expf(-(1.0f/(sigmax*sigmay))*(x-meanx)*(y-meany));
}

inline float diemDROI::gaussDist(float x, float meanx, float sigmax)
{
	return (1.0f/sqrtf(TWO_PI*sigmax*sigmax))*expf(-(1.0f/(sigmax*sigmax))*(x-meanx)*(x-meanx));
}

void diemDROI::drawHistogram(IplImage * src)
{
	//Create a window
	cvNamedWindow("RGB_histogram", CV_WINDOW_AUTOSIZE);

	//Create Skeletons
	IplImage *R_plane = cvCreateImage( cvSize(mov.width,mov.height), 8, 1);
	IplImage *G_plane = cvCreateImage( cvSize(mov.width,mov.height), 8, 1);
	IplImage *B_plane = cvCreateImage( cvSize(mov.width,mov.height), 8, 1);

	int bin = 51;
	float ranges0 [] = {0, 255};
	float *ranges[] = {ranges0};

	IplImage *rhist_img = cvCreateImage( cvSize(320,240), 8, 4);

	CvHistogram *rhist = cvCreateHist(1, &bin, CV_HIST_ARRAY, ranges, 1);
	CvHistogram *ghist = cvCreateHist(1, &bin, CV_HIST_ARRAY, ranges, 1);
	CvHistogram *bhist = cvCreateHist(1, &bin, CV_HIST_ARRAY, ranges, 1);


	float rmax_value, gmax_value, bmax_value; // Max, pixel numbers in a bin
	int bin_w, i, j;

	cvSplit(src, B_plane, G_plane, R_plane, 0);
	R_plane->origin = src->origin;
	G_plane->origin = src->origin;
	B_plane->origin = src->origin;

	cvCalcHist( &R_plane, rhist, 0, NULL );
	cvCalcHist( &G_plane, ghist, 0, NULL );
	cvCalcHist( &B_plane, bhist, 0, NULL );

	//Get the maximum number of pixels in a histogram bar
	cvGetMinMaxHistValue( rhist, 0, &rmax_value, 0, 0 );
	cvGetMinMaxHistValue( ghist, 0, &gmax_value, 0, 0 );
	cvGetMinMaxHistValue( bhist, 0, &bmax_value, 0, 0 );

	//scale the histogram bar heights to fit into the histogram image window
	cvScale( rhist->bins, rhist->bins, ((double)rhist_img->height)/rmax_value, 0 );
	cvScale( ghist->bins, ghist->bins, ((double)rhist_img->height)/gmax_value, 0 );
	cvScale( bhist->bins, bhist->bins, ((double)rhist_img->height)/bmax_value, 0 );

	//Background of histogram window is set to black
	cvSet( rhist_img, cvScalarAll(0), 0 );

	//width of each bin is same for all the three histograms R, G, B
	bin_w = cvRound((double)rhist_img->width/bin);

	for( i = 0; i < (bin-1); i++ )
	{
		j = bin - i;

		/*if(0)
		{
			cvRectangle( rhist_img, cvPoint(i*bin_w, rhist_img->height),
				cvPoint((i+1)*bin_w, rhist_img->height - cvRound(cvGetReal1D(rhist->bins,i))),
				cvScalar(255,0,0,20), -1, 1, 0 );

			cvRectangle( rhist_img, cvPoint(i*bin_w, rhist_img->height),
				cvPoint((i+1)*bin_w, rhist_img->height - cvRound(cvGetReal1D(ghist->bins,i))),
				cvScalar(0,255,0,20), 1, 1, 0 );

			cvRectangle( rhist_img, cvPoint(i*bin_w, rhist_img->height),
				cvPoint((i+1)*bin_w, rhist_img->height - cvRound(cvGetReal1D(bhist->bins,i))),
				cvScalar(0,0,255,20), -1, 1, 0 );
		}*/
		if(1)
		{
			cvLine( rhist_img, cvPoint(j*bin_w, rhist_img->height - cvRound(cvGetReal1D(rhist->bins,i))),
				cvPoint((j-1)*bin_w, rhist_img->height - cvRound(cvGetReal1D(rhist->bins,i+1))),
				cvScalar(255,0,0,20), 2, 8, 0);

			cvLine( rhist_img, cvPoint(j*bin_w, rhist_img->height - cvRound(cvGetReal1D(ghist->bins,i))),
				cvPoint((j-1)*bin_w, rhist_img->height - cvRound(cvGetReal1D(ghist->bins,i+1))),
				cvScalar(0,255,0,20), 2, 8, 0);

			cvLine( rhist_img, cvPoint(j*bin_w, rhist_img->height - cvRound(cvGetReal1D(bhist->bins,i))),
				cvPoint((j-1)*bin_w, rhist_img->height - cvRound(cvGetReal1D(bhist->bins,i+1))),
				cvScalar(0,0,255,20), 2, 8, 0);
		}

	}

	cvShowImage( "RGB_histogram", rhist_img );   

	cvReleaseImage(&rhist_img);
	cvReleaseImage(&R_plane);
	cvReleaseImage(&G_plane);
	cvReleaseImage(&B_plane);
	cvReleaseHist(&rhist);
	cvReleaseHist(&ghist);
	cvReleaseHist(&bhist);
}

#ifndef _USEUNSTABLE
void diemDROI::importFromIAS()
{
	vector<string> garbage_lines;
	string line;

	// open the ias file
	string filename = myOpenIASFileDialog();
	ifstream in_file;
	in_file.open(filename.c_str(), ifstream::in);

	// remove the header which should appear as something like:
	/* # Interest area created ...
	   #  blah blah
	   #  blah
	   #  b

	   0 -33 ....
	   -34 -66 ...
	   -67 -99 ...
	   .
	   .
	   .
	*/
	while(getline(in_file, line))
	{
		// if there is no header, then trouble...
		if(line[0] != '#')
			break;
	}

	while(!in_file.eof())
	{
		// get the input data in the correct format
		// id must start with 1
		string start_offset, end_offset, 
			type, id, 
			left, top, right, bottom, name;
		int _start_offset, _end_offset, _id, _left, _top, _right, _bottom;
		in_file >> start_offset >> end_offset >> type >> id >> left >> top >> right >> bottom >> name;
		istringstream instr;
		instr.str(start_offset); instr >> _start_offset; instr.clear();
		instr.str(end_offset); instr >> _end_offset; instr.clear();
		instr.str(id); instr >> _id; instr.clear();
		instr.str(left); instr >> _left; instr.clear();
		instr.str(top); instr >> _top; instr.clear();
		instr.str(right); instr >> _right; instr.clear();
		instr.str(bottom); instr >> _bottom; instr.clear();
		
		if(name.length())
		{
			// do stuff with it
			// I need to set the GUI objects, as well as the Interactive Objects...
			// I should really have made a class that encapsulates both
			// But... 
			// Too late now.

			// milliseconds per frame
			float MSpF = 1000.f/(float)FPS;
			// Convert the ms offsets to frame numbers
			int frame_number = round(-(float)_start_offset / MSpF);

			// Get the bounds into the object structure

			// See if the droi already exists
			int i = 0;
			_id = gui_Objects.size();
			for(vector<pkmDROI *>::iterator itr=objs.begin(); itr!=objs.end(); ++itr, i++)
			{
				if((*itr)->my_name == name)
					_id = i;
			}

			// See if it is a new ID
			if(_id >= gui_Objects.size())
			{
				int idx = total_gui_elements + gui_Objects.size() + 1;
				// add the object gui with the name from the input
				ofxGuiFiles *obj = (ofxGuiFiles *)panel5->addFiles(idx, "", 
					string(name).length()*7.5, OFXGUI_FILES_HEIGHT, name, "", "");
				obj->mState = 0;
				gui_Objects.push_back(obj);

				// setup the DROI
				pkmDROI *o = new pkmDROI();
				//if(parameterId == gui_NewFHObjectTrigger)
				//	o->setup(0, 0, mov.width, mov.height, numFrames, 1, string(name));
				//else	
				// only rectangles for now
					o->setup(0, 0, mov.width, mov.height, numFrames, 2, string(name));
				o->setRecordState(0);
				objs.push_back(o);
				current_obj = gui_Objects.size() - 1;

				// add a new tracker in case the user wants to track with boosting
				//pkmDROITracker *t = NULL;
				pkmBoostingTracker *t = NULL;
				trackers.push_back(t);
				loading_obj = false;
			}
			else
			{
				// gui_Objects[id];
			}
			// update the object
			objs[_id]->setFrameWithoutRecord(frame_number);
			objs[_id]->setVerticesToRectangle(_left-offset_x, _top-offset_y, _right-_left+1, _bottom-_top+1);
		}
	}

}
#else
#endif








// use the proper cvconvexhull method to find the object mask... use something like roitomask?
// use the boosting trackers...

// lemeur
// tatler in scenes