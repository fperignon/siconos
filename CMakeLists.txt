#===============================================================================
# cmake utility to compile and install siconos.
#
#
# Siconos development team - September 2015.
#
#===============================================================================

# ============= Global 'standard' cmake Settings =============

# Set minimum version for cmake
# We advise 3.12 at least, but 3.5 is authorized since it's the default version on current debian stable.
cmake_minimum_required(VERSION 3.5)

# Set cmake modules directory (i.e. the one which contains all user-defined FindXXX.cmake files among other things)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Force out-of-source build
if ("${CMAKE_SOURCE_DIR}" MATCHES "${CMAKE_BINARY_DIR}")
  message (SEND_ERROR "In source building not supported (not recommanded indeed). You need to :
    * cleanup your source directory :  \"rm -rf ./CMakeFiles/ ./CMakeCache.txt\"
    * try configure process again in a new directory
    (e.g. mkdir <anywhere>/build ; cd <anywhere>/build ; cmake ${CMAKE_SOURCE_DIR}) ...")
  return()
endif()

# If not given, turn build type to release mode.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release." FORCE)
endif()

# Usefull configuration scripts, written in python and called by cmake.
configure_file(scripts/buildtools.py share/buildtools.py COPYONLY)

set(PROJECT_NAME siconos)

# Get current year (for licence and other cartridges)
execute_process(COMMAND date "+\"%Y\"" OUTPUT_VARIABLE YEAR)
string(STRIP ${YEAR} YEAR)

# Read user option file, if provided on command line
if(USER_OPTIONS_FILE)
  if(NOT IS_ABSOLUTE ${USER_OPTIONS_FILE})
    get_filename_component(USER_OPTIONS_FILE ${USER_OPTIONS_FILE} REALPATH
      BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  message("\n !!!!! Load user-defined options set from file ${USER_OPTIONS_FILE} !!!!! \n")

  # Configure to take into account any change in ${USER_OPTIONS_FILE}
  configure_file(${USER_OPTIONS_FILE} current_options.cmake COPYONLY)
  include(${CMAKE_CURRENT_BINARY_DIR}/current_options.cmake)
else()
  message("\n !!!!! Load default configuration set from file ${CMAKE_SOURCE_DIR}/cmake/default_options.cmake !!!!! \n")
  include(default_options)
endif()

option(WITH_OPENMP "Use OpenMP" OFF)  # Move this to use options file? 
option(WITH_CCACHE "Use ccache" OFF)

# Set target names from components list
if(NOT COMPONENTS)
  message(WARNING "Components list is empty - Nothing will be built.")
endif()

set(AVAILABLE_COMPONENTS "externals numerics kernel control mechanics mechanisms io")
foreach(component_name ${COMPONENTS})
  # Check and set components list
  string(TOUPPER ${component_name} upname)
  list(FIND ${AVAILABLE_COMPONENTS} ${component_name} comp_exists)
  if(NOT comp_exists)
    message(FATAL_ERROR "Unknown siconos component : ${component_name}")
  endif()
  set(HAS_SICONOS_${upname} TRUE CACHE INTERNAL "True if the corresponding component is processed.")
endforeach()

include(SiconosTools)
# ============= The project =============
# Set project name and project languages
# Languages:
# - Fortran is optional, needed by ql0001, lsodar, hem5 solvers.
# - CXX is required for kernel and following components
# - C always required

if(WITH_CXX)
  project(${PROJECT_NAME} CXX)
  # Set CXX compilation flags
  include(CXXCompilerSetup) 
#else()
  # we may need to compile only numerics and only in C.
  # Really???
  project(${PROJECT_NAME} C)
  # Set C compilation flags
  include(CCompilerSetup)
endif()

# Activate fortran, if needed.
if(WITH_FORTRAN)
  enable_language(Fortran)
  include(FCompilerSetup)
  set(HAS_FORTRAN TRUE)
endif()

# Common setup
include(SiconosSetup)

# ========= Search for 'common' dependencies =========
include(siconos_find_common_dep)

# ========= Python bindings =========
if(WITH_PYTHON_WRAPPER)
  add_subdirectory(wrap)
endif()

# ========= Tests setup =========
if(WITH_TESTING)
  include(CTest)
  include(SiconosCTest)
  # log file for tests setup
  file(WRITE ${CMAKE_BINARY_DIR}/tests.log "--- Siconos tests setup --- \n")
endif()

# ========= Documentation setup =========
# This includes doxygen_warnings,
# doxygen doc from source files, sphinx doc
# and docstrings from doxygen, depending on the
# enabled options.
add_subdirectory(docs)

# ========= Setup each required component =========
foreach(COMPONENT ${COMPONENTS})
  message("\n--------------------------------------------------")
  add_subdirectory(${COMPONENT})
endforeach()

if(WITH_TESTING)
  # -- Help tests to find libraries at runtime.
  include(tools4tests)
  set_ldlibpath()
endif()

# -- SiconosConfig.h generation --
# Warning : depends on find_package(...) results.
if(EXISTS ${CMAKE_SOURCE_DIR}/config.h.cmake)
  configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/SiconosConfig.h)
  if(${CMAKE_VERSION} VERSION_GREATER "3.14")
    install(FILES ${CMAKE_BINARY_DIR}/SiconosConfig.h TYPE INCLUDE)
  else()
    install(FILES ${CMAKE_BINARY_DIR}/SiconosConfig.h DESTINATION include)
  endif()
endif()

finalize_doc()

include(FeatureSummary)
feature_summary(WHAT ALL)

set(ConfigPackageLocation lib/cmake/siconos-${SICONOS_VERSION})
# =========== Generate siconos scripts ===========
if(EXISTS ${CMAKE_SOURCE_DIR}/scripts/CMakeLists.txt.in)
  configure_file(scripts/CMakeLists.txt.in scripts/CMakeLists-temp.txt @ONLY)
  configure_file(${CMAKE_BINARY_DIR}/scripts/CMakeLists-temp.txt ${CMAKE_BINARY_DIR}/scripts/CMakeLists.txt @ONLY)
  install(FILES ${CMAKE_BINARY_DIR}/scripts/CMakeLists.txt DESTINATION ${ConfigPackageLocation})
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/scripts/siconos.py.in)
  message("-- Generate siconos script ...")
  configure_file(scripts/siconos.py.in scripts/siconos @ONLY)
  install(PROGRAMS ${CMAKE_BINARY_DIR}/scripts/siconos DESTINATION bin)
endif()

# ============= Siconos Package configuration file =============
# https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#creating-packages
# 
include(CMakePackageConfigHelpers)
# Generate siconos-config.cmake
configure_package_config_file(siconos-config.cmake.in ${CMAKE_BINARY_DIR}/siconos-config.cmake
  INSTALL_DESTINATION ${ConfigPackageLocation})

# install(FILES ${CMAKE_BINARY_DIR}/SiconosConfig.h DESTINATION include/${PROJECT_NAME})

# Generate siconos-config-version.cmake file.
write_basic_package_version_file(
  "${CMAKE_BINARY_DIR}/siconos-config-version.cmake"
  VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}
  COMPATIBILITY ExactVersion
  )


export(EXPORT siconosTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/siconosTargets.cmake"
  NAMESPACE Siconos::
  )

install(EXPORT siconosTargets
  NAMESPACE Siconos::
  DESTINATION ${ConfigPackageLocation}) 


# install config files
install(
  FILES ${CMAKE_BINARY_DIR}/siconos-config.cmake ${CMAKE_BINARY_DIR}/siconos-config-version.cmake
  DESTINATION ${ConfigPackageLocation})

# ============= Summary =============
message(STATUS "\n============================================ Summary ============================================")
message(STATUS "${PROJECT_NAME} version ${SICONOS_VERSION} is now ready for compilation and installation.")
message(STATUS "To proceed run 'make' and 'make install' and optionaly 'make test'.")
message(STATUS "C++ Compiler : ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ flags : ${CMAKE_CXX_FLAGS}")
message(STATUS "C Compiler : ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "C flags : ${CMAKE_C_FLAGS}")
message(STATUS "Fortran Compiler : ${CMAKE_Fortran_COMPILER} ${CMAKE_Fortran_COMPILER_VERSION}")
message(STATUS "Fortran flags : ${CMAKE_Fortran_FLAGS}")
message(STATUS "Compilation mode is : ${CMAKE_BUILD_TYPE}")
message(STATUS "Code Sources are in : ${CMAKE_SOURCE_DIR}")
message(STATUS "Blas is ${WITH_BLAS}. ")
message("    Blas libraries : ${BLAS_LIBRARIES}.")
message("    Blas headers : ${BLAS_HEADER} in ${BLAS_INCLUDE_DIRS}.")
message(STATUS "Lapack is ${WITH_LAPACK}.")
message("    Lapack libraries : ${LAPACK_LIBRARIES}.")
message("    Lapack headers : ${LAPACK_HEADER} in ${LAPACK_INCLUDE_DIRS}.")
if (USE_SYSTEM_SUITESPARSE)
  message(STATUS "CXSparse headers : ${CXSparse_INCLUDE_DIR}")
  message(STATUS "CXSparse library : ${CXSparse_LIBRARY}")
else()
  message(STATUS "CSparse headers : externals/SuiteSparse/CSparse")
endif()
message(STATUS "Python interpreter is ${Python3_EXECUTABLE}")
if(WITH_PYTHON_WRAPPER)
  message(STATUS "Python libraries are ${Python3_LIBRARIES}")
endif()
message(STATUS "The project will be installed in ${CMAKE_INSTALL_PREFIX}")
if(WITH_PYTHON_WRAPPER)
  message(STATUS "${SICONOS_PYTHON_PACKAGE} python package will be installed in ${SICONOS_PYTHON_INSTALL_DIR}")
endif()
message(STATUS "To get more information about dependencies, config or else, ")
message(STATUS "check CMakeCache.txt file or re-run cmake with -DPRINT_ENV=ON.")
message(STATUS "=================================================================================================\n")

# Log build info into ${CMAKE_BINARY_DIR}/Testing/Notes/Build
write_notes()

