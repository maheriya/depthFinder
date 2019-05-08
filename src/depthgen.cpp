/*g
 * depthgen.cpp
 *
 *  Created on: May 06, 2019
 *  Description: Generates depth from given left and right views
 */
#include <depthgen.hpp>

#define EN_RECTIFICATION 1

static const double ms_mult            = (1000/getTickFrequency());
static const string intrinsic_filename = DFINDER_DATA_DIR "/cal1/intrinsics.yml";
static const string extrinsic_filename = DFINDER_DATA_DIR "/cal1/extrinsics.yml";

int DepthGen::init() {
    width      = IMG_WIDTH;
    height     = IMG_HEIGHT;
    roi_width  = IMG_WIDTH;
    roi_height = IMG_HEIGHT;

#if EN_RECTIFICATION
    // Read the Intrinsic and extrinsic files, and load the required matrices
    // reading intrinsic parameters
    FileStorage fs(intrinsic_filename.c_str(), FileStorage::READ);
    if (!fs.isOpened()) {
        printf("Failed to open calibration file %s\n", intrinsic_filename.c_str());
        return -1;
    }

    //-////////////////////////////////////////////////////////////////////////
    // Rectification prep
    //-////////////////////////////////////////////////////////////////////////
    fs["M1"] >> M1;
    fs["D1"] >> D1;
    fs["M2"] >> M2;
    fs["D2"] >> D2;

    fs.open(extrinsic_filename.c_str(), FileStorage::READ);
    if (!fs.isOpened()) {
        printf("Failed to open calibration file %s\n", extrinsic_filename.c_str());
        return -1;
    }

    fs["R"] >> R;
    fs["T"] >> T;

    //-////////////////////////////////////////////////////////////////////////
    //                     vvvv  Rectification prep        vvvv
    // Need to do once per session. We can save this along
    // with calibration also. However, it is convenient here as some parameters
    // like alpha can be changed as required.
    int64 t = getTickCount();
    Size img_size(width, height); // we don't have an image yet, and hence, use predefined numbers

    // Create rectification matrices (R1, R2, P1, P2, Q)
    stereoRectify(M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, 0, -1, img_size, &roi1, &roi2);
    //stereoRectify(M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, img_size, &roi1, &roi2);

    initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_16SC2, map11, map12);
    initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_16SC2, map21, map22);

    //-//////////////////////////////////////////////////////////////////////////////////
    // Find common_roi, such that we get valid pixels in both left and right images.
    //
    //-//////////////////////////////////////////////////////////////////////////////////
    // For common_roi, we have three choices:
    // #1: Use intersection of left and right roi. This gives the biggest possible image
    //     common_roi = Rect(max(roi1.x, roi2.x), max(roi1.y, roi2.y), min(roi1.width, roi2.width), min(roi1.height, roi2.height));
    //
    // #2: Use intersection of l/r roi only to find top-left coordinate, and then use fixed width x height:
    //     common_roi = Rect(max(roi1.x, roi2.x), max(roi1.y, roi2.y), roi_width, roi_height);
    //
    // #3: Use fixed width x height and get centered roi:
    //     common_roi = Rect((width-roi_width)/2, (height-roi_height)/2, roi_width, roi_height);
    //
    // All of the three methods above have been tried and shown to be working. However,
    // currently, #3 is being used.
    //-//////////////////////////////////////////////////////////////////////////////////
    //
    common_roi = Rect((width - roi_width) / 2, (height - roi_height) / 2, roi_width, roi_height);
    //common_roi = Rect(max(roi1.x, roi2.x), max(roi1.y, roi2.y), min(roi1.width, roi2.width), min(roi1.height, roi2.height));


#if DEBUG>=1
    printf("Rectification prep: roi1(%d, %d, %d, %d), roi2(%d, %d, %d,%d)\n", roi1.x, roi1.y, roi1.width, roi1.height, roi2.x, roi2.y,
            roi2.width, roi2.height);
    printf("Rectification prep: %fms\n", (getTickCount() - t) * ms_mult);
#endif

    //                     ^^^^ end of rectification prep  ^^^^
    //-////////////////////////////////////////////////////////////////////////
#endif // EN_RECTIFICATION
    return 0;
}

void DepthGen::rectify(Mat& l, Mat& r, Mat& oimgl, Mat& oimgr) {
#if EN_RECTIFICATION
#if DEBUG>=2
    int64 t = getTickCount();
#endif
    //-////////////////////////////////////////////////////////////////////////
    // Rectification -- final remapping per frame
    Mat imgl = l.clone();
    Mat imgr = r.clone();
    remap(imgl, imgl, map11, map12, INTER_LINEAR);
    remap(imgr, imgr, map21, map22, INTER_LINEAR);
    // Get RoI from L and R images
    oimgl = imgl(common_roi);
    oimgr = imgr(common_roi);
    rectangle(oimgl, roi1, Scalar(20, 10, 250), 2, LINE_AA);
    rectangle(oimgr, roi2, Scalar(20, 10, 250), 2, LINE_AA);

#if DEBUG>=2
    t = getTickCount() - t;
    printf("Rectification: Time elapsed: %5.2fms\n", t * ms_mult);
#endif
#else // !EN_RECTIFICATION
    oimgl = imgl;
    oimgr = imgr;
#endif // EN_RECTIFICATION
}


void DepthGen::triangulate(vector<Point2d>& lpoints, vector<Point2d>& rpoints, vector<Point3d>& points3D) {
    vector<Point2d> unlpoints, unrpoints;
    undistortPoints(lpoints, unlpoints, M1, D1, R1, P1);
    undistortPoints(rpoints, unrpoints, M2, D2, R2, P2);
#if DEBUG>=1
    // Add points to the images as circles
    for(auto const& point: lpoints) {
        drawCircle(imgl, point.x, point.y, 5, 0); // unrectified left image
    }
    for(auto const& point: rpoints) {
        drawCircle(imgr, point.x, point.y, 5, 0); // unrectified right image
    }
    for(auto const& point: unlpoints) {
        drawCircle(imgl_recti, point.x, point.y, 5, 1); // rectified left image
    }
    for(auto const& point: unrpoints) {
        drawCircle(imgr_recti, point.x, point.y, 5, 1); // rectified right image
    }
    cout << "lpoints = \n "<< lpoints << endl;
    cout << "rpoints = \n "<< rpoints << endl;
    cout << "unlpoints = \n "<< unlpoints << endl;
    cout << "unrpoints = \n "<< unrpoints << endl;
#endif

    Mat points4D;
    triangulatePoints(P1, P2, unlpoints, unrpoints, points4D);
#if DEBUG>=2
    cout << "points4D = \n "<< points4D << endl;
#endif
    convertPointsFromHomogeneous(points4D.t(), points3D);
#if DEBUG>=1
    cout << "points3D = \n "<< points3D << endl;
#endif
}


void DepthGen::setMode(algo_t _algo) {
    algo = _algo;
}


int DepthGen::proc(Mat& _imgl, Mat& _imgr, Mat& oimgl, Mat& oimgr, Mat& disp) {
    int64 t = getTickCount();


    t = getTickCount();
    // imgl and imgr must be rectified here: TBD
    imgl = _imgl;
    imgr = _imgr; // we are only copying references. For debug
    rectify(imgl, imgr, oimgl, oimgr);
    // For visual debug, keep references available within the class
    imgl_recti = oimgl;
    imgr_recti = oimgr;
    if (algo == dSGBM) {
        if (stereo_sgbm(oimgl, oimgr, disp) != 0) {
            std::cout << "SGBM failed" << std::endl;
            return -1;
        }
    } else if (algo == dStereoBM) {
        if (stereo_bm(oimgl, oimgr, disp) != 0) {
            std::cout << "StereoBM failed" << std::endl;
            return -1;
        }
    }
#if DEBUG>1
    t = getTickCount() - t;
    printf("Disparity: Time elapsed: %5.2fms\n", t * 1000 / getTickFrequency());
#endif

    return 0;
}


int DepthGen::stereo_bm(Mat& _imgLeft, Mat& _imgRight, Mat& disp) {
    Mat imgLeft, imgRight;
    cvtColor(_imgLeft, imgLeft, CV_BGR2GRAY);
    cvtColor(_imgRight, imgRight, CV_BGR2GRAY);
    Mat imgDisparity16S = Mat(imgLeft.rows, imgLeft.cols, CV_16S);

    if (imgLeft.empty() || imgRight.empty()) {
        std::cout << " --(!) Error reading images " << std::endl;
        return -1;
    }

    if (!initialized) {
        //stbm->setROI1(NULL);
        //stbm->setROI2(NULL);
        stbm->setPreFilterCap(PreFilterCap);
        stbm->setBlockSize(SADWindowSize);
        stbm->setMinDisparity(MinDisparity);
        stbm->setNumDisparities(NumDisparities);
        stbm->setTextureThreshold(TextureThreshold);
        stbm->setUniquenessRatio(UniquenessRatio);
        stbm->setSpeckleWindowSize(SpeckleWindowSize);
        stbm->setSpeckleRange(SpeckleRange);
        stbm->setDisp12MaxDiff(Disp12MaxDiff);
        initialized = true;
    }
    stbm->compute(imgLeft, imgRight, imgDisparity16S);

    // Display as a CV_8UC1 image
    //imgDisparity16S.convertTo(disp, CV_8UC1, SCALE_16S_TO_8U); // disp will have range 0-255
    double alpha, beta;
#if 1
    {
        double min, max;
        minMaxLoc(imgDisparity16S, &min, &max);
        printf("========= stereo_bm:\n\tmax(disp): %3.1f,\n\tmin(disp): %3.1f\n", max, min);
        //--double max=52.0;
        //--double min=-2.0;
        alpha = 255.0/(max-min);
        beta = -255.0*min/(max-min);
        printf("\tAlpha: %7.3f,\n\tBeta: %7.3f\n", alpha, beta);
        // For conversion, use alpha (multipliers) and beta (addition)
        // Equations are: 255.0/(max-min), -255.0*min/(max-min)
        // max(disp): 2288.0,
        // min(disp): -784.0
        // Alpha:        0.083,
        // Beta:        65.078
    }
#endif
    imgDisparity16S.convertTo(disp, CV_8U, alpha, beta); // disp will have range 0-255
    return 0;
}

int DepthGen::stereo_sgbm(Mat& imgLeft, Mat& imgRight, Mat& disp) {
    int max_disp = NumDisparities;
    int wsize;

    wsize = SADWindowSize/4;
    wsize = (wsize%2) ? wsize:wsize+1;

    if (imgLeft.empty() || imgRight.empty()) {
        std::cout << " --(!) Error reading images " << std::endl;
        return -1;
    }

    Mat imgLeft_scaled, imgRight_scaled;
    Mat left_disp;
    if (max_disp <= 0 || max_disp % 16 != 0) {
        std::cout << "Incorrect max_disparity value: it should be positive and divisible by 16\n";
        return -1;
    }
    if (wsize <= 0 || wsize % 2 != 1) {
        std::cout << "Incorrect window_size value: it should be positive and odd\n";
        return -1;
    }
    if (StereoDownscale) {
        // downscale the views to speed-up the matching stage, as we will need to compute both left
        // and right disparity maps for confidence map computation
        max_disp /= 2;
        if (max_disp % 16 != 0)
            max_disp += 16 - (max_disp % 16);
        cv::resize(imgLeft, imgLeft_scaled, Size(), 0.5, 0.5);
        cv::resize(imgRight, imgRight_scaled, Size(), 0.5, 0.5);
    } else {
        imgLeft_scaled = imgLeft.clone();
        imgRight_scaled = imgRight.clone();
    }

    if (!initialized) {
        sgbm->setMinDisparity(0);
        sgbm->setNumDisparities(max_disp);
        sgbm->setBlockSize(wsize);
        sgbm->setPreFilterCap(PreFilterCap);
        sgbm->setP1(8 * wsize * wsize); // was 24
        sgbm->setP2(32 * wsize * wsize); // was 96
        sgbm->setUniquenessRatio(UniquenessRatio);
        sgbm->setSpeckleWindowSize(SpeckleWindowSize);
        sgbm->setSpeckleRange(SpeckleRange);
        sgbm->setDisp12MaxDiff(Disp12MaxDiff);
        sgbm->setMode(StereoSGBM::MODE_SGBM);
        initialized = true;
    }

    sgbm->compute(imgLeft_scaled, imgRight_scaled, left_disp);

    if (StereoDownscale) {
        // upscale raw disparity back for a proper comparison:
        cv::resize(left_disp, left_disp, Size(), 2.0, 2.0);
        left_disp = left_disp * 2.0;
    }

    // convert to CV_8UC1 image
    left_disp.convertTo(disp, CV_8UC1, SCALE_16S_TO_8U); // disp will have range 0-255

    return 0;
}


void DepthGen::drawCircle(Mat& img, int npx, int npy, int r, int iobj) {
    // Draw a circle on image
    if (iobj == 0)
        circle(img, Point(npx, npy), r, Scalar(250, 20, 10), -1); // blue
    else
        circle(img, Point(npx, npy), r, Scalar(20, 250, 250), -1); // yellow
}


void DepthGen::stop(void) {
    //
}

