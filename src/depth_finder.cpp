/*
 * depth_finder.cpp
 *
 *  Created on: May 03, 2019
 */

#include "depth_finder.hpp"

#define DEBUG              1


#include <chrono>            // c++11 std timer functions
//#include <thread>            // c++11 std threads
#include <iterator>
//-//////////////////////////////////////////////////////////////////////////////////////
// Boost related
//-//////////////////////////////////////////////////////////////////////////////////////
//#include <boost/asio.hpp>
//#include <boost/bind.hpp>
//#include <boost/asio/steady_timer.hpp>
//#include <boost/program_options.hpp>
//namespace po = boost::program_options;

#define ISEMPTY(img) (img.size().width==0 || imgl.size().height==0)


using namespace cv;
using namespace std;

//-//////////////////////////////////////////////////////////////////////////////////////
// image holders
//-//////////////////////////////////////////////////////////////////////////////////////
static Mat imgl, imgr;  // left and right captured images; output of grabber
static Mat disp;        // disparity image; output of proc
static Mat rlimg;       // concatenated right+left image

int main(int argc, char **argv) {
    cout << "Done!" << endl;
    return (EXIT_SUCCESS);
}

