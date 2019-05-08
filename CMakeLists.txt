##================================================================
##
##================================================================
cmake_minimum_required(VERSION 3.8)
project( depth_finder CXX )

set(DFINDER_MAJOR_VERSION 1)
set(DFINDER_MINOR_VERSION 2)
set(DFINDER_PATCH_VERSION 1)
set(DFINDER_VERSION "${DFINDER_MAJOR_VERSION}.${DFINDER_MINOR_VERSION}.${DFINDER_PATCH_VERSION}")
include(cmake/ConfDirs.cmake)

INCLUDE(FindPkgConfig)

#-####################################################################################
# Data directory. Change this if you want to run depth_finder from local build/data
# "${INSTALL_CMAKE_DIR}/data" is by default "/usr/local/data"
#-####################################################################################
set (DFINDER_DATA_DIR "./data")


# Configure a header file to pass settings to source code
configure_file (
  "${PROJECT_SOURCE_DIR}/include/depthFinderConfig.h.in"
  "${PROJECT_BINARY_DIR}/depthFinderConfig.h"
)


# Add the binary tree to the search path for include files - mainly headers
include_directories(include ${PROJECT_BINARY_DIR})

# Add the source files
file(GLOB SOURCES_CPP "src/*.cpp")
file(GLOB SOURCES_C   "src/*.c")

# Find OpenCV
find_package( OpenCV 3.4.2 EXACT REQUIRED ) # not requiring EXACT sub-version
set(CFLGS  "-DOPENCV")

# Add Executable
if (EN_CUDA)  ## CUDA is required
  CUDA_ADD_EXECUTABLE(depth_finder ${SOURCES_CPP} ${SOURCES_C})
else()
  ADD_EXECUTABLE(depth_finder ${SOURCES_CPP} ${SOURCES_C})
endif()


#SET(EXTRA_CFLAGS "-fpermissive")
SET(CMAKE_CXX_FLAGS "${CFLGS} ${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -std=c++11" )
SET(CMAKE_C_FLAGS "${CFLGS} ${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}" )

target_link_libraries(${PROJECT_NAME}
	${OpenCV_LIBS}
	-lboost_system -lboost_thread -lboost_program_options
	-lreadline -pthread)

install(TARGETS ${PROJECT_NAME}
  EXPORT DFINDERTargets
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin)

install(DIRECTORY data DESTINATION ${INSTALL_CMAKE_DIR})

## Generate and install CMake Config files for use by other CMake builds
include(cmake/GenConfig.cmake)

#EOF
