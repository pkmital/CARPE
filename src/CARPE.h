// Written by Parag K Mital for the DIEM project
// Started 1 Nov, 2008 
// Last updated 18 May, 2010

// UNSTABLE components include dynamic regions of interest with 
// tracking from optical flow and boosting trackers.  
// A user interface has been built into CARPE UNSTABLE allowing users to 
// draw regions and track them by hand (e.g. holding the mouse down 
// and leetting the movie play, moving the dROI as the movie plays) or
// by using the optical flow calculations or boosting tracking methods.
//
// These methods require:
//		CUDA libraries (tested on 2.1), 
//		SiftGPU (see my google code),
//		FlowLib (http://gpu4vision.icg.tugraz.at/index.php?content=downloads.php)
//		Boosting Tracker (not included but contact me if you are interested).

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

#define _USEUNSTABLE 0

#ifndef _USEUNSTABLE
	#pragma comment(lib, "src/CUDA/lib/cufft.lib")
	#pragma comment(lib, "src/CUDA/lib/cublas.lib")
	#pragma comment(lib, "src/CUDA/lib/cuda.lib")
	#pragma comment(lib, "src/CUDA/lib/cudart.lib")
	#pragma comment(lib, "src/SIFTGPU/SiftGPU/lib/SIFTGPU.lib")
	#pragma comment(lib, "src/SIFTGPU/SiftGPU/lib/cg.lib")
	#pragma comment(lib, "src/FlowLib/lib/vm.lib")
	#pragma comment(lib, "src/FlowLib/lib/common_static.lib")
	#pragma comment(lib, "src/FlowLib/lib/flow.lib")


	#include "GPUFlow.h"
	#include "pkmDROITracker.h"
	#include "pkmBoostingTracker.h"
	#include "pkmDROI.h"
	#include "pkmActiveContourSeg.h"
#endif

#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <vector>

#include "VarFlow.h"

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxGuiTypes.h"
#include "ofQtVideoSaver.h"
#include "ofVideoPlayer.h"
//#include "pkmEXTAudioFileReader.h"
#include <ofxOpenCv.h>
#include <opencv2/opencv.hpp>
using namespace cv;
// GUI stuff:

const int total_gui_elements = 100;
const string cluster_tags[] = {"1", "2", "3", "4", "8", "1-2", "1-4", "1-8", "2-4", "4-8"};
const string scalar_tags[] = {"1","2","4","6","8"};
const string res_tags[] = {"640x480","800x600","1024x768","1280x960","1440x900","1600x1200"};


class diemDROI : public ofBaseApp, public ofxGuiListener 
{

#ifndef _USEUNSTABLE
	enum { cam_shift, dense_optical_flow, boosting };
#endif

enum
{
	gui_Loader,
	gui_LoaderSlider,

	gui_ObjectsPanel,
	gui_NewFHObjectTrigger,
	gui_NewRectObjectTrigger,

	gui_OptionsPanel,
	gui_EnableItemsPanel,
	gui_EnableEyes,
	gui_EnableSaccades,
	gui_EnableSubjectNames,
	gui_EnableAllFixations,
	gui_EnableMeanBinocular,
	gui_EnableMovie,
	gui_EnableHistogram,
	gui_EnableAlphaScreen,
	gui_EnableClustering,
	gui_EnableHeatmap,
	gui_EnableContours,
	gui_EnableNormalized,
	gui_EnableRealTime,
	gui_CovarianceSwitch,
	gui_EyefilesSwitch,

	gui_ExportPanel,
	gui_EnableRecording,
	gui_EnableSaveMovieImages,
	gui_ImportFromIAS,
	gui_ExportAllObjects,

	gui_TrackPanel,
	gui_TrackObjectBtn,
	gui_TrackAdaptiveBtn,
	gui_BinDROIs,
	gui_InitDROIs,
	gui_TrackCAMShift,
	gui_TrackFlow,
	gui_TrackBoosting,
	
	gui_MovieTimeHolder,
	gui_MovieTimeSlider,

	gui_ScalarsSwitch,
	gui_SigmaSwitch,
	gui_ResSwitch,
	gui_ClustersSwitch,

	gui_VisualizationPanel,
	gui_DrawOpticalFlowBtn,
	gui_DrawFlicker,
	gui_DrawEdges
};

public:
	//////////////////////////////
	diemDROI();
	~diemDROI();
	//////////////////////////////

	//////////////////////////////
	void update();
	//////////////////////////////
		void updateEyesForCurrentFrame();
		void updateEdges();
		void updateFlicker();
		void updateColorFlow();
#ifndef _USEUNSTABLE
		void trackObject();
#endif
	//////////////////////////////
		
	//////////////////////////////
	void draw();
	//////////////////////////////
		void drawEyes();
		void drawMovieControls();
		void drawEdges();
		void drawFlicker();
//#ifndef _USEUNSTABLE
		void drawColorFlow();
//#endif
		void drawHistogram(IplImage *src);
	//////////////////////////////
		
	//////////////////////////////
	void setup();
	//////////////////////////////
        void loadEyeTrackingMovie();
        void loadEyeTrackingData();
        void loadEyeTrackingAudio();
		void initializeOptions();
		void initializeGui();
		void initializeMovieOutput();
        void initializeOpticalFlow();
	//////////////////////////////
	
	//////////////////////////////
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased();
	//////////////////////////////
	
	// load the ias file into a vector of objects...
	void importFromIAS();

	//////////////////////////////
	void handleGui(int paramterId, int task, void* data, int length);
	ofxGui*					gui;
	ofxGuiPanel*			panel1;
	ofxGuiPanel*			panel2;
	ofxGuiPanel*			panel3;
	ofxGuiPanel*			panel4;
	ofxGuiPanel*			panel5;
	ofxGuiPanel*			panel6;
	//////////////////////////////
	
	//////////////////////////////
	int						frameCounter;		// garbage variable for the text input
	float					movie_time;			// for drawing the movie time
	string					movie_name;			// for loading the movie and eye data
	ofVideoPlayer			mov;				// the movie

	int						movhr, 
							movmin, 
							movsec;				// printing the time
	bool					isPaused;			// are we paused
	bool					updatedFrame;		// did the frame change
	//////////////////////////////

	
	//////////////////////////////
	ofstream				filePtr;			// for writing stats to a file
	//////////////////////////////
	
	//////////////////////////////
	float					movieSpeed;			// current movie speed
	int						frameNumber;		// current frame number
	int						lastFrameNumber;	// previous frame number
	int						FPS, numFrames;		// fps.. total frames
	//////////////////////////////
	
	//////////////////////////////
	ofxDirList				myDIR;				// defaults to the directory ./eyefiles
	//////////////////////////////

	int						mouse_state_down;	// is the mouse pressed down?

#ifndef _USEUNSTABLE
	//////////////////////////////
	bool					newObjectTrigger;	// new droi
	vector<ofxGuiFiles *>	gui_Objects;		// this displays the objects as a selectable list
	vector<pkmDROI *>		objs;				// all the objects
	int						current_obj;		// index of current obj
	bool					loading_obj;		// ensure that you can only load one object at a time
	//////////////////////////////

	//////////////////////////////
	ofxGuiButton *			trackButton;		// callback to the object track button
	bool					trackCurrentObject; // are we tracking?
	int						tracker_type;		// deprecated
	bool					useAdaptTrack;		// type of tracker to use
	bool					binDROIs;			// when true, record objects are set to 0 vertices
	//////////////////////////////



	//////////////////////////////
	vector<pkmBoostingTracker *> trackers;
	pkmActiveContourSeg segmenter;
	//////////////////////////////
#endif

    
	//////////////////////////////
	ofxCvColorImage			movCvImg;			// used for the RGB histogram
	//////////////////////////////



	// keeps x,y,dil data on each eye file
	class eye
	{
	public:
		// ptr our eye-tracking files
		ifstream			eyeFile;
		// values for coordinates
		vector<float>		eye_x,		eye_y;
		vector<float>		b_eye_x,	b_eye_y;			// the prefix 'b_' for the binocular eye
		vector<float>		m_eye_x,	m_eye_y;			// the prefix 'm_' for the mean of the mono and bino eye
		vector<float>		dilation,	b_dilation, m_dilation;
		vector<int>			left_event,	right_event;
		vector<bool>		is_fixation;
		string				prefix;							// subject prefix from filename
	};	
	eye						*eyeList;
	int						eyeListSize, b_eyeListSize, m_eyeListSize;		// number of eye files for the current frame w/o blinks
	int						maxEyeListSize;									// because of blinks, eyeListSize may be smaller

	int						*xs, *b_xs, *m_xs;
	int						*ys, *b_ys, *m_ys;				// dynamic storage to plot x,y
	int						*prev_m_xs, *prev_m_ys;			// previous mean x,y
	float					*scale_eyes;						// enlarge the eyes as they stay in the same area
	
	double					*inputModelMap;		// for the clustering algorithm
	enum					{COV_SPHERE, COV_DIAG, COV_GEN};
	int						cov_type;
	
	bool					loadedFiles;		// flag for when the eye-tracking files are finished loading
	bool					doneRecording;		// if the movie is over
	
	int						eyeCounter;			// counter for eyeListSize

	int						offset_x, offset_y; // movie offset based on screen res
	int						map_scalar;
	
	ofTexture				heatmap_tex;		// displaying a heatmap
	ofTexture				movieAndAlpha;		// alpha heatmap
	ofTexture				alphaScreen;

	ofxCvGrayscaleImage 	heatmap_sc, 
                            heatmap;			// drawing the opengl version of the heatmap
	ofxCvColorImage			heatmap3;			// drawing the opengl version of the colorized heatmap

	unsigned char*			eyePtsMap;			// original eye locations
	unsigned char*			colorized;			// colorized heatmap image
	unsigned char*			rgb;				// rgb triple from the colormap computation
	unsigned int*			unnormalized;		// unnormalized heatmap image
	unsigned char*			movieOut;			// image for recording the movie output
	
	ofQtVideoSaver			saver;				// for recording the output movie
	ofImage					saverImg;			// for saving the movie as images
	
	ofxCvGrayscaleImage		red, green, blue;
	
	ofxCvContourFinder		me;					// contour finder
	
	GLuint*					pboIds;				// IDs of PBOs
	
	float gaussDist(int &x,int mean);
	float gaussDist(float x, float y, float meanx, float meany, float sigmax, float sigmay);
	float gaussDist(float x, float meanx, float sigmax);
	float sigma;
	float A;
	float B;
	
	// variables for drawing
	bool					showEyes, showMovie, showHeatmap, showRecording, showAlphaScreen,
							showContours, showNormalized, showRealTime, showClustering, showHistogram, 
							showMeanBinocular, showSaccades, showAllFixations, showSubjectNames;

    
	bool					drawOpticalFlow;	// are we drawing the optical flow?
	ofxCvColorImage			motion_img;
    bool					enableMotion;
#ifndef _USEUNSTABLE
    //////////////////////////////
	GPUFlow *				myflowlib;			// calculates optical flow
	GPUFlow *				frameflowlib;		// drawing the whole frame's motion
	//////////////////////////////
#else
    ofxCvGrayscaleImage     motion_previous_previous_img, motion_previous_img, motion_this_img;
    ofxCvFloatImage         motion_x_img, motion_y_img, motion_dist_img;
    VarFlow                 *opticalFlow;
    bool                    bSaved;
#endif


	bool					showFlicker;
	ofxCvGrayscaleImage		flicker_img;
	ofxCvGrayscaleImage		flicker_this_img;
	ofxCvGrayscaleImage		flicker_prev_img;

	bool					showEdges;
	ofxCvGrayscaleImage		edge_img;

	bool					loadBinocular, 
							saveMovieImages;


	//const cluster_tags[] = {"1", "2", "4", "6", "8", "1-2", "1-4", "1-8", "2-4", "4-8"};
	//panel1->addSwitch(gui_ClustersSwitch, "Clustering Kernels", 
	int minClusterComponents, maxClusterComponents;


    //pkmEXTAudioFileReader   *audioFileReader;
    int                     audioFrameSize;
    
	// detects if the gui changed, and to update the display
	bool					reset;
	
	bool					bSetup, bVBOSupported;
};

