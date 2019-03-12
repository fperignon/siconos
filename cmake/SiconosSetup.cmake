# ===================================================
# Set main parameters for the Siconos cmake project
#
# ===================================================
include(SiconosTools)

# =========== Windows stuff ... ===========
include(WindowsSiconosSetup)

# --------- CMake project internal variables ---------

# Siconos current version
include(SiconosVersion)

# File used to print tests setup messages.
set(TESTS_LOGFILE ${CMAKE_BINARY_DIR}/tests.log)

# get system architecture 
# https://raw.github.com/petroules/solar-cmake/master/TargetArch.cmake
include(TargetArch)
target_architecture(SYSTEM_ARCHITECTURE) # This variable seems to be required
# to set CMAKE_SWIG_FLAGS in wrap/CMakeLists.txt.

if(WITH_SYSTEM_INFO) # User defined option, default = off
  include(CMakePrintSystemInformation)
  message(STATUS "SYSTEM ARCHITECTURE: ${SYSTEM_ARCHITECTURE}")
endif()

# extensions of headers files that must be taken into account
set(HDR_EXTS h hpp)

# To include or not unstable source files
set(WITH_UNSTABLE FALSE)

# dirs of 'local' headers. Must be filled by each component.
set(${PROJECT_NAME}_LOCAL_INCLUDE_DIRECTORIES
  CACHE INTERNAL "Local include directories.")

set(SICONOS_INCLUDE_DIRECTORIES
  CACHE INTERNAL "Include directories for external dependencies.")

set(SICONOS_LINK_LIBRARIES
  CACHE INTERNAL "List of external libraries.")

set(${PROJECT_NAME}_LOCAL_LIBRARIES
  CACHE INTERNAL "List of siconos components libraries.")

set(installed_targets ${installed_targets}
  CACHE INTERNAL "List of installed libraries for the siconos project.")

set(PRIVATE PRIVATE CACHE INTERNAL "")
set(PUBLIC PUBLIC CACHE INTERNAL "")

set(tests_timeout 120 CACHE INTERNAL "Limit time for tests (in seconds)")

# extensions of source files that must be taken into account
get_standard_ext()
set(SRC_EXTS ${ALL_EXTS})

if(WITH_GIT) # User defined option, default = off
  # Check if git is available
  # and get last commit id (long and short).
  # Saved in SOURCE_ABBREV_GIT_SHA1 and SOURCE_GIT_SHA1
  # These vars are useful for tests logs and 'write_notes' macro.
  find_package(Git)
  if(GIT_FOUND)
    set(CTEST_GIT_COMMAND "${GIT_EXECUTABLE}" )     
    execute_process(COMMAND 
      ${GIT_EXECUTABLE} log -n 1 --pretty=format:%h 
      OUTPUT_VARIABLE SOURCE_ABBREV_GIT_SHA1
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    
    execute_process(COMMAND 
      ${GIT_EXECUTABLE} log -n 1 --pretty=format:%H
      OUTPUT_VARIABLE SOURCE_GIT_SHA1
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  endif()
endif()

# Save date/time into BUILD_TIMESTAMP var
string(TIMESTAMP BUILD_TIMESTAMP)

# ---- PYTHON SETUP ----
# We force Python3!
if(${CMAKE_VERSION} VERSION_GREATER "3.12.3")
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  if(WITH_PYTHON_WRAPPER)
    find_package(Python3 COMPONENTS Development REQUIRED)
  endif()
else()
  # Our FindPythonFull is just a copy of the one distributed
  # with cmake > 3.12.
  find_package(PythonFull COMPONENTS Interpreter REQUIRED)
  if(WITH_PYTHON_WRAPPER)
    find_package(PythonFull COMPONENTS Development REQUIRED)
  endif()
endif()

if(WITH_PYTHON_WRAPPER OR WITH_DOCUMENTATION)
  include(FindPythonModule)
  # --- xml schema. Used in tests. ---
  if(WITH_XML)
    set(SICONOS_XML_SCHEMA "${CMAKE_SOURCE_DIR}/kernel/swig/SiconosModelSchema-V3.7.xsd")
    if(NOT NO_RUNTIME_BUILD_DEP)
      find_python_module(lxml REQUIRED)
    endif()
  endif()
endif()

# --- End of python conf ---

# Choice of CSparse/CXSparse integer size
# Note FP :  this should be in externals isn't it?
IF(NOT DEFINED SICONOS_INT64)
  IF(NOT SIZE_OF_CSI)
    INCLUDE(CheckTypeSize)
    CHECK_TYPE_SIZE("size_t" SIZE_OF_CSI)
    IF(NOT SIZE_OF_CSI)
      message(FATAL_ERROR
        "Could not get size of size_t, please specify SICONOS_INT64.")
    ENDIF(NOT SIZE_OF_CSI)
  ENDIF(NOT SIZE_OF_CSI)

  IF ("${SIZE_OF_CSI}" EQUAL 8)
    SET(SICONOS_INT64 TRUE)
  ELSE ("${SIZE_OF_CSI}" EQUAL 8)
    SET(SICONOS_INT64 FALSE)
  ENDIF ("${SIZE_OF_CSI}" EQUAL 8)
ENDIF()


# =========== install setup ===========

# Provides install directory variables as defined by the GNU Coding Standards.
include(GNUInstallDirs)  # It defines CMAKE_INSTALL_LIBDIR
# Set prefix path for libraries installation
# --> means that any library target will be installed
# in CMAKE_INSTALL_PREFIX/_install_lib
if(${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(_install_lib ${${PROJECT_NAME}_INSTALL_LIB_DIR})
else()
  ASSERT(CMAKE_INSTALL_LIBDIR)
  set(_install_lib ${CMAKE_INSTALL_LIBDIR})
  set(${PROJECT_NAME}_INSTALL_LIB_DIR ${_install_lib})
endif()


# --- RPATH stuff ---
# Warning: RPATH settings must be defined before install(...) settings.
# Source : https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling

if(FORCE_SKIP_RPATH) # Build with no RPATH. Do we really need this option??
  set(CMAKE_SKIP_BUILD_RPATH TRUE)
else()
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
endif()

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

# when building a binary package, it makes no sense to add this rpath
if(NOT FORCE_SKIP_RPATH)
  # the RPATH to be used when installing
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif(NOT FORCE_SKIP_RPATH)

# don't add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif()

# List of cmake macros that will be distributed with siconos software.
# They may be required during call to siconos script
# or to configure projects depending on Siconos (e.g. siconos-tutorials)
set(cmake_macros
  SiconosTools.cmake
  # FindQGLViewer.cmake
  FindPythonModule.cmake
  valgrind.supp
  )

foreach(file IN LISTS cmake_macros)
  install(FILES cmake/${file} DESTINATION share/siconos/cmake)
endforeach()

# =========== uninstall target ===========
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

if(WITH_PYTHON_WRAPPER)
  # deal with files installed for python 
  add_custom_target(uninstall
    echo >> ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt
    COMMAND cat ${CMAKE_CURRENT_BINARY_DIR}/python_install_manifest.txt >> ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
else()
  add_custom_target(uninstall
    echo >> ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# init all common options for enabled components
set(common_options DOCUMENTATION TESTING UNSTABLE PYTHON_WRAPPER
  DOXYGEN_WARNINGS DOXY2SWIG)
foreach(opt ${common_options})
  init_to_default_option(${opt})
endforeach()
# ========= Documentation =========
if(WITH_DOCUMENTATION OR WITH_DOXY2SWIG OR WITH_DOXYGEN_WARNINGS)
  set(USE_DOXYGEN TRUE)
endif()

if(WITH_DOCUMENTATION)
  set(USE_SPHINX TRUE)
endif()


# =========== use ccache if available ===========
if(WITH_CCACHE)
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
  endif()
endif()

