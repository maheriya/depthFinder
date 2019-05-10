/*
 * common.hpp
 *
 *  Created on: May 03, 2019
 */
#ifndef __DEPTH_COMMON_H__
#define __DEPTH_COMMON_H__

#include "depthFinderConfig.h"  // From the build directory
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <ctype.h>
#include <iostream>


#define ST_BM              0
#define ST_SGBM            1


#define STEREO_ALGO        ST_SGBM // ST_BM or ST_SGBM

#define DEBUG              1

#define USE_GPU            1
#define IMG_RESIZE         1280
#define IMG_WIDTH          1280
#define IMG_HEIGHT         1024



#if DEBUG>=1
 #define SHOW_OWINDOW       1  // Original images
 #define SHOW_RWINDOW       1  // Rectified image
 #define SHOW_DWINDOW       1  // Disparity image
#else
 #define SHOW_OWINDOW       0  // Original image
 #define SHOW_RWINDOW       0  // Rectified image
 #define SHOW_DWINDOW       0  // Disparity image
#endif

//
#define STR_OWindow        "Original";
#define STR_RWindow        "Rectified";
#define STR_DWindow        "Disparity";


//-////////////////////////////////////////////////////////////////////////////
// Disparity calculation parameters
#define SAD_BLOCK_SIZE     15
#define NUM_DISPARITIES    144
#define MIN_DISP          -64
#define SCALE_16S_TO_8U    (0.0714) // 256 / (16 * (NUM_DISPARITIES - MIN_DISP))
// Threshold under which to ignore disparity. Not used.
#define MIN_DISP_THRESH    4

#if SHOW_OWINDOW || SHOW_RWINDOW || SHOW_DWINDOW
#define GUI_ENABLED 1
#else
#define GUI_ENABLED 0
#endif


#endif // __DEPTH_COMMON_H__
