/*
 * depth_finder.cpp
 *
 *  Created on: May 03, 2019
 */

#include "depth_finder.hpp"


#include <chrono>            // c++11 std timer functions
//#include <thread>            // c++11 std threads
#include <iterator>
//-//////////////////////////////////////////////////////////////////////////////////////
// Boost related
//-//////////////////////////////////////////////////////////////////////////////////////
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#define ISEMPTY(img) (img.size().width==0 || imgl.size().height==0)


using namespace cv;
using namespace std;

//-//////////////////////////////////////////////////////////////////////////////////////
// image holders
//-//////////////////////////////////////////////////////////////////////////////////////
static Mat imgl, imgr;  // left and right captured images; output of grabber
static Mat disp;        // disparity image; output of proc
static Mat rlimg_orig, rlimg_rectified;       // concatenated right+left image

static const String OWindow = STR_OWindow;
static const String RWindow = STR_RWindow;
static const String DWindow = STR_DWindow;

static void show_images() {

#if GUI_ENABLED
#if SHOW_OWINDOW
    namedWindow(OWindow, CV_WINDOW_AUTOSIZE);
#endif
#if SHOW_RWINDOW
    namedWindow(RWindow, CV_WINDOW_AUTOSIZE);
#endif
#if SHOW_DWINDOW
    namedWindow(DWindow, CV_WINDOW_AUTOSIZE);
#endif

#if SHOW_OWINDOW
    if (!ISEMPTY(rlimg_orig))
        imshow(OWindow, rlimg_orig);       // original r+l images
#endif
#if SHOW_RWINDOW
    if (!ISEMPTY(rlimg_rectified))
        imshow(RWindow, rlimg_rectified);  // rectified r+l images
#endif
#if SHOW_DWINDOW
    if (!ISEMPTY(disp))
        imshow(DWindow, disp); // disparity image
#endif
    char key = waitKey();
    if (key == 'w') {
        cout << "Writing images images\n";
        imwrite("original.png", rlimg_orig);
        imwrite("rectified.png", rlimg_rectified);
        imwrite("disparity.png", disp);
    }
    destroyAllWindows();
#endif  // GUI_ENABLED
}

int main(int argc, char **argv) {
    // Parse arguments
    bool playback = true;
    string left = "";
    string right = "";
    int mode = 1;
    try {
        po::options_description generic("depth_finder options");
        generic.add_options()
            ("help", "Get this help message")
            ("mode",
                    po::value<int>(&mode)->default_value(1),
                    "Disparity calculation mode: 0: StereoBM, 1: SGBM")
            ;

        po::options_description hidden("Backend options");
        hidden.add_options()
                ("left",
                        po::value<vector<string>>(),
                        "Name of the left image file")
                ("right",
                        po::value<vector<string>>(),
                        "Name of the right image file")
                ;

        // Group all
        po::options_description all("All allowed options");
        all.add(generic).add(hidden);

        // Group visible
        po::options_description visible("");
        visible.add(generic);

        po::positional_options_description p;
        p.add("left", 1).add("right", 1); // to add other positional argument, add more. e.g., p.add("input-file", 1).p.add("output-file", 1), etc.

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(all).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << "Usage: depth_finder [options] <left image filename> <right image filename>" << endl;
            cout << visible << "\n";  // only show visible options
            return 0;
        }
        if (vm.count("left")) { // only one file name allowed
            left = po::validators::get_single_string(vm["left"].as<vector<string>>(), false);
        }
        if (vm.count("right")) { // only one file name allowed
            right = po::validators::get_single_string(vm["right"].as<vector<string>>());
        }
        if (vm.count("left") && vm.count("right")) {
            cout << "Left and right files: " << left << ", " << right << endl;
        } else {
            cout << "Please specify left and right image file names to use" << endl;
            return 1;
        }
    }
    catch (exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        cerr << "Options parsing exception of unknown type!\n";
    }

    DepthGen depthgen;
    if (mode == ST_SGBM) {
        cout << "Setting SGBM mode\n";
        depthgen.setMode(DepthGen::dSGBM);
    } else { // ST_BM
        cout << "Setting StereoBM mode\n";
        depthgen.setMode(DepthGen::dStereoBM);
    }
    depthgen.init();

    // Grab images
    imgl = imread(left,  CV_LOAD_IMAGE_COLOR);   // Read the left image
    imgr = imread(right, CV_LOAD_IMAGE_COLOR);   // Read the right image
    if (! imgl.data ) {
        cout <<  "Could not open or find the image " << left << std::endl;
        return -1;
    }
    if (! imgr.data ) {
        cout <<  "Could not open or find the image " << right << std::endl;
        return -1;
    }

    // Process
    Mat imgl_rectified, imgr_rectified;
    depthgen.proc(imgl, imgr, imgl_rectified, imgr_rectified, disp);


    // Find world coordinates from image correspondences
    vector<Point2d> lpoints;
    vector<Point2d> rpoints;
    vector<Point3d> points3D;
    // Left  R0: 439,527    647,509    841,497 (Actual: 440,528     645,511    840,497)
    // Right R0: 443,507    646,515    849,522
    // Left  R1: 447,604    684,588    899,567 (Actual: 450,605     682,590    898,568)
    // Right R1: 380,578    605,594    837,600
    // L+R Coordinate 1: R0,C0             // L+R Coordinate 2: R0,C1             // L+R Coordinate 3: R0,C2
    lpoints.push_back(Point2d(439.,527.)); lpoints.push_back(Point2d(647.,509.)); lpoints.push_back(Point2d(841.,497.));
    rpoints.push_back(Point2d(443.,507.)); rpoints.push_back(Point2d(646.,515.)); rpoints.push_back(Point2d(849.,522.));

    // L+R Coordinate 4: R0,C0             // L+R Coordinate 5: R0,C1             // L+R Coordinate 6: R0,C2
    lpoints.push_back(Point2d(447.,604.)); lpoints.push_back(Point2d(684.,588.)); lpoints.push_back(Point2d(899.,567.));
    rpoints.push_back(Point2d(380.,578.)); rpoints.push_back(Point2d(605.,594.)); rpoints.push_back(Point2d(837.,600.));

    depthgen.triangulate(lpoints, rpoints, points3D);

    hconcat(imgr, imgl, rlimg_orig); // r+l -> easier to see 3D with crossed eyes
    hconcat(imgr_rectified, imgl_rectified, rlimg_rectified);
    // Show
    cout << "Showing images...\n";
    show_images();
    cout << "Done!" << endl;
    return (EXIT_SUCCESS);
}












