/*
 *  VarFlowExtras.h
 *  avSaliency
 *
 *  Created by Mr. Magoo on 8/30/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include <opencv2/opencv.hpp>

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265
#endif

void drawColorField(IplImage* imgU, IplImage* imgV, IplImage* imgColor);