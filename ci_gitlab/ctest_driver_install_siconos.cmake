#-----------------------------------------------
# Ctest driver for siconos install.
# Target : continuous integration on gitlab-ci,
# aims at providing a proper install of siconos
#
#
# Input variables :
# - SICONOS_INSTALL_DIR : where to install siconos. Default : ../install-siconos
# - USER_FILE : user option file used by cmake to configure siconos. Default : siconos_default.cmake.
#   Warning : always searched in ci_gitlab directory.
# - OSNAME : host system name (used to qualify cdash build). If not set, try to catch info
#   using common commands (lsb_release ...)
# ----------------------------------------------

# Assumes :
# - 'siconos' git repository is available in ../

# used as CMAKE_INSTALL_PREFIX
if(NOT SICONOS_INSTALL_DIR)
  set(SICONOS_INSTALL_DIR ../install-siconos/)
endif()


# With MAIN_SOURCE_DIR the path to git repository for siconos tutorial
get_filename_component(MAIN_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR} DIRECTORY) # path to this (ctest_driver) file
# -- job : build and install siconos --
message("--- Start conf for siconos install.")
# - Source dir and path to siconos install
# We assume CI setup, with build dir in git repository and
# The 'cmake' source dir is ..
if(NOT CTEST_SOURCE_DIRECTORY)
  set(CTEST_SOURCE_DIRECTORY ..)
endif()

# Build name (for cdash)
if(NOT CTEST_BUILD_NAME)
  include(${CTEST_SOURCE_DIRECTORY}/cmake/SiconosVersion.cmake)
  set(CTEST_BUILD_NAME "Siconos install (${SICONOS_VERSION}-devel, ref commit : ${CI_COMMIT_SHORT_SHA})")
  if(EXTRA_NAME)
    set(CTEST_BUILD_NAME "${CTEST_BUILD_NAME} - ${EXTRA_NAME}.")
  endif()
endif()

if(USER_FILE)
  set(SICONOS_CMAKE_OPTIONS -DUSER_OPTIONS_FILE=${USER_FILE})
endif()

list(APPEND SICONOS_CMAKE_OPTIONS -DCMAKE_INSTALL_PREFIX=${SICONOS_INSTALL_DIR})
list(APPEND SICONOS_CMAKE_OPTIONS -DCMAKE_CXX_STANDARD=11)
list(APPEND SICONOS_CMAKE_OPTIONS -DSICONOS_USE_BOOST_FOR_CXX11=OFF)
if(DEFINED ENV{OCE_INSTALL}) # set if oce has been installed using oce repo, in install_oce.sh
  message("Search oce in $ENV{OCE_INSTALL}.")
  list(APPEND SICONOS_CMAKE_OPTIONS -DOCE_DIR=$ENV{OCE_INSTALL})
endif()
set(current_project siconos_install)
# Parallel build only for siconos_install. For examples it leads to: warning: jobserver unavailable: using -j1. Add `+' to parent make rule.
set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE ${CTEST_SOURCE_DIRECTORY}/cmake/valgrind.supp)

include(${MAIN_SOURCE_DIR}/ci_gitlab/ctest_common.cmake)
