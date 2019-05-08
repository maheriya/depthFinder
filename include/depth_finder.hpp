/*
 * depth_finder.hpp
 *
 *  Created on: May 03, 2019
 */
#ifndef __DEPTH_FINDER_H__
#define __DEPTH_FINDER_H__

#include "common.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "depthgen.hpp"

//--template<typename Duration = std::chrono::milliseconds>
//--struct measure {
//--    template<typename F, typename ...Args>
//--    static typename Duration::rep execution(F&& func, Args&&... args) {
//--        auto start = std::chrono::steady_clock::now();
//--        std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
//--        auto duration = std::chrono::duration_cast<Duration> (std::chrono::steady_clock::now() - start);
//--        return duration.count();
//--    }
//--
//--    template<typename F, typename ...Args>
//--    static Duration duration(F&& func, Args&&... args) {
//--        auto start = std::chrono::steady_clock::now();
//--        std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
//--        auto duration = std::chrono::duration_cast<Duration> (std::chrono::steady_clock::now() - start);
//--        return duration;
//--    }
//--
//--};

#endif // __DEPTH_FINDER_H__
