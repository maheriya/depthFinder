/*
 * depthgen.hpp
 *
 *  Created on: May 06, 2019
 *  Description: Depth generator. Generates depth from given left and right views.
 */
#ifndef __DEPTHGEN_H__
#define __DEPTHGEN_H__

#include "common.hpp"


#include <opencv2/opencv.hpp>
#include "opencv2/calib3d/calib3d.hpp"

using namespace cv;
using namespace std;

static const int  minDisparity      = 0;
static const int  numDisparities    = NUM_DISPARITIES;
static const int  blockSize         = SAD_BLOCK_SIZE;


static Ptr<StereoSGBM> sgBM = StereoSGBM::create(
    minDisparity,
    numDisparities,
    blockSize
    );

static Ptr<StereoBM> stBM = StereoBM::create(numDisparities, blockSize);

class DepthGen {
public:

    typedef enum {dStereoBM = 0, dSGBM = 1, dStereoCNN = 2} algo_t;
    // Constructor
    DepthGen() :
        algo(dStereoBM),
        stbm(stBM),
        sgbm(sgBM),
        initialized(false) {
    }

    // Init
    int  init();
    int  proc(Mat& imgl, Mat& imgr, Mat& oimgl, Mat& oimgr, Mat& disp);
    void stop(void);

    void setMode(algo_t algo);
    void rectify(Mat& imgl, Mat& imgr, Mat& oimgl, Mat& oimgr);
    // Destructor
    ~DepthGen() {
        this->stop();
    }
    void triangulate(vector<Point2d>& lpoints, vector<Point2d>& rpoints, vector<Point3d>& points3D);
    void drawCircle(Mat& img, int npx, int npy, int r, int iobj);

private:
    Ptr<StereoBM>     stbm;
    Ptr<StereoSGBM>   sgbm;
    struct timespec   tm; // for getting return values

    algo_t            algo;
    bool              initialized;

    // Parameters
    static const int  PreFilterSize     = 5;
    static const int  PreFilterCap      = 63;
    static const int  SADWindowSize     = SAD_BLOCK_SIZE;
    static const int  MinDisparity      = MIN_DISP;
    static const int  NumDisparities    = NUM_DISPARITIES;
    static const int  UniquenessRatio   = 14;
    static const int  SpeckleWindowSize = 50;
    static const int  SpeckleRange      = 1;
    static const int  TextureThreshold  = 0;
    static const int  Disp12MaxDiff     = 1;
    //
    static const bool StereoDownscale   = true;  // downscale input before running SGBM

    // For rectification
    Mat               M1, D1, M2, D2;  // intrinsics
    Mat               R, T;            // extrinsics
    Mat               Q;               // disparity-to-depth map matrix.
    Mat               R1, P1, R2, P2;  // rotation (R*) and projection (P*) matrices
    int               roi_width;
    int               roi_height;
    int               width;
    int               height;
    Mat               imgl;
    Mat               imgr;
    Mat               imgl_recti, imgr_recti;     // references to current left and right rectified images. For visual debug.
    Mat               map11, map12, map21, map22; // Post-rectification mapping matrices
    Rect              roi1, roi2;
    Rect              common_roi; // common_roi is the intersection of roi1 and roi2: guarantees valid roi in BOTH images


    int stereo_bm(Mat& l, Mat& r, Mat& d);
    int stereo_sgbm(Mat& l, Mat& r, Mat& d);

};



#endif // __DEPTHGEN_H__
