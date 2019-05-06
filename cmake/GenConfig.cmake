# Install depth_finder and DFINDERConfig*.cmake files
# ========================================
# Add all targets to the build-tree export set
export(TARGETS ${PROJECT_NAME} FILE "${PROJECT_BINARY_DIR}/DFINDERTargets.cmake")
 
# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE DFINDER)
 
# Create the DFINDERConfig.cmake and DFINDERConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
set(CONF_INCLUDE_DIRS include)
set(CONF_LIBNAME "${PROJECT_NAME}")
set(CONF_LIBRARY_DIRS lib)
configure_file(DFINDERConfig.cmake.in "${PROJECT_BINARY_DIR}/DFINDERConfig.cmake" @ONLY)
set(CONF_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/${CONF_INCLUDE_DIRS}")
set(CONF_LIBRARY_DIRS "${CMAKE_INSTALL_PREFIX}/${CONF_LIBRARY_DIRS}")
configure_file(DFINDERConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DFINDERConfig.cmake" @ONLY)
configure_file(DFINDERConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/DFINDERConfigVersion.cmake" @ONLY)
 
# Install the DFINDERConfig.cmake and DFINDERConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DFINDERConfig.cmake"
  "${PROJECT_BINARY_DIR}/DFINDERConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)


# Install the export set for use with the install-tree
install(EXPORT DFINDERTargets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
