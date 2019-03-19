include(SiconosTools)

#====================================================================
#
# Define and build a target for the current component
#
# Usage:
#
# library_project_setup(COMPONENT)
#
# The following variables must be properly set:
# - <COMPONENT>_DIRS : sources directories
# - <COMPONENT>_EXCLUDE_SRCS : list of files to exclude from build process
#   for this component.
#
# This function 
#   creates a target <component> from all sources files in <component>_DIRS
#   excluding files from <component>_EXCLUDE_SRCS
#
function(create_siconos_component COMPONENT)

  # --- Collect source files from given directories ---
  # --> set ${COMPONENT}_SRCS
  get_sources(${COMPONENT})
  # Create the library
  if(BUILD_SHARED_LIBS AND NOT BUILD_${COMPONENT}_STATIC)
    add_library(${COMPONENT} SHARED ${${COMPONENT}_SRCS})
  else()
    add_library(${COMPONENT} STATIC ${${COMPONENT}_SRCS})
    set_property(TARGET ${COMPONENT} PROPERTY POSITION_INDEPENDENT_CODE ON)
  endif()

  # Append component source dirs to include directories
  # (Private : only to build current component).
  foreach(dir IN LISTS ${COMPONENT}_DIRS)
    target_include_directories(${COMPONENT} PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir}>)
  endforeach()

  # Add current component include dirs that should be propagated through
  # interface, in build tree.
  # WARNING : includes for install interface are handled later
  # in component install, and may be different.
  foreach(dir IN LISTS ${COMPONENT}_INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${COMPONENT} INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir}>)
  endforeach()
    
  include(SiconosVersion)

  set_target_properties(${COMPONENT} PROPERTIES
    OUTPUT_NAME "siconos_${COMPONENT}"
    VERSION "${SICONOS_SOVERSION}"
    SOVERSION "${SICONOS_SOVERSION_MAJOR}")
  
  list(APPEND installed_targets ${COMPONENT})
  set(installed_targets ${installed_targets}
    CACHE INTERNAL "List of all exported components (targets).")
  
endfunction()


function(siconos_component_install_setup COMPONENT)
  # libraries
  install(TARGETS ${COMPONENT}
    EXPORT siconosTargets
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION include
    )

  # Required for SiconosConfig.h
  target_include_directories(${COMPONENT} INTERFACE
    $<INSTALL_INTERFACE:include>)

  # Setup include dirs for install interface
  # Must be relocatable, i.e. no abs path!
  foreach(dir IN LISTS ${COMPONENT}_INSTALL_INTERFACE_INCLUDE_DIRECTORIES)

    if(${CMAKE_VERSION} VERSION_GREATER "3.12.0")
      file(GLOB _headers CONFIGURE_DEPENDS
        LIST_DIRECTORIES false ${_FILE} ${dir}/*.h ${dir}/*.hpp)
    else()
      file(GLOB _headers
        LIST_DIRECTORIES false ${_FILE} ${_FILE} ${dir}/*.h ${dir}/*.hpp)
    endif()
    list(APPEND _all_headers ${_headers})
    # Append component source dirs to include directories
    # for targets linked to current target from install path.
    # target_include_directories(${COMPONENT} INTERFACE
    #   $<INSTALL_INTERFACE:include/${COMPONENT}/${dir}>)
    
    # And each include path in install interface must obviously be installed ...
    # Note FP :  maybe we should have an explicit list of headers to be installed,
    # for each component, instead of a list of dirs?
    # ---- Installation ---
  endforeach()
  
  # Do not install files listed in ${COMPONENT}_EXCLUDE_HDRS
  if(_all_headers)
    foreach(_file IN LISTS ${COMPONENT}_EXCLUDE_HDRS)
      list(REMOVE_ITEM  _all_headers "${CMAKE_CURRENT_SOURCE_DIR}/${_file}")
    endforeach()
  endif()

  if(_all_headers)
    install(
      FILES ${_all_headers}
      DESTINATION include/${COMPONENT}
      )
    
    target_include_directories(${COMPONENT} INTERFACE
      $<INSTALL_INTERFACE:include/${COMPONENT}>)
  endif()
  

endfunction()


#   include(doc_tools)
#   # --- doxygen warnings ---
#   include(doxygen_warnings)

#   # --- documentation ---
  
#   # xml files for python docstrings ...
#   # xml files are required to build docstrings target
#   # and so they must be build during cmake run.
#   if(WITH_PYTHON_WRAPPER)
#     include(swig_python_tools)
#     doxy2swig_docstrings(${COMPONENT})
#   endif()
  
#   # update the main doxy file, without building the doc
#   if(WITH_${COMPONENT}_DOCUMENTATION)
#     update_doxy_config_file(${COMPONENT})

#     # Prepare target to generate rst files from xml
#     doxy2rst_sphinx(${COMPONENT})
#   endif()

#   list(APPEND installed_targets ${COMPONENT})
#   list(REMOVE_DUPLICATES installed_targets)
#   set(installed_targets ${installed_targets}
#     CACHE INTERNAL "Include directories for external dependencies.")

#   # windows stuff ...
#   include(WindowsLibrarySetup)
#   windows_library_extra_setup(siconos_${COMPONENT} ${COMPONENT})
  
#   if(BUILD_SHARED_LIBS)
#     if(LINK_STATICALLY) # static linking is a nightmare
#       set(REVERSE_LIST ${${COMPONENT}_LINK_LIBRARIES})
#       LIST(REVERSE REVERSE_LIST)
#       target_link_libraries(${COMPONENT} ${PRIVATE} ${REVERSE_LIST})
#     endif()
#   endif()    

#   # --- python bindings ---
#   if(WITH_${COMPONENT}_PYTHON_WRAPPER)
#     add_subdirectory(swig)
#   endif()
  
#   if(WITH_PYTHON_WRAPPER)
#     add_dependencies(${COMPONENT} ${COMPONENT}_docstrings)
#   endif()
  
# endmacro()

