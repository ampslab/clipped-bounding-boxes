CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(DeadSpace)

SET(MYLIB "deadspace")

################################################################################
# Advanced VTK stuff
# E.g., cmake needs to be run as:
#   cmake -DVTK_DIR:PATH=~/Software/VTK-7.1.0/VTK-build -DDEBUG_MODE=ON ../
################################################################################
#find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})

################################################################################
# GTest
################################################################################
ADD_SUBDIRECTORY ( googletest-v1.7.0 )
enable_testing()
include_directories( ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} )

################################################################################
# The Computational Geometry Algorithms Library
################################################################################
find_package(CGAL QUIET)
if ( CGAL_FOUND )
  include( ${CGAL_USE_FILE} )
else()
  message(STATUS "The CGAL library not found, few (optional) apps won't be compiled.")
endif()
################################################################################

OPTION(DEBUG_MODE "Build the project using debugging code." OFF)
OPTION(MAKE_EXECUTABLES "Make the executables." ON)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

IF(DEBUG_MODE)
  ADD_DEFINITIONS("-O0 -g -Wall -std=c++11")
  SET(EXECUTABLE_OUTPUT_PATH ../debug/)
ELSE(DEBUG_MODE) # -Winline
  ADD_DEFINITIONS("-O3 -Wall -std=c++11 -DNDEBUG -DCGAL_NDEBUG")
  SET(EXECUTABLE_OUTPUT_PATH ../bin/)
ENDIF(DEBUG_MODE)
MESSAGE (STATUS "DEBUG_MODE (${DEBUG_MODE}): ${EXECUTABLE_OUTPUT_PATH}")

############## SETTING DIRECTORY PATHS #################
SET(LIBRARY_OUTPUT_PATH lib/)
SET(SRC_DIR src/)
SET(APP_DIR apps/)
SET(INC_DIR include/)
SET(TST_DIR src/tests/)
FILE(GLOB SRC_FILES  ${SRC_DIR}/*.cc)
FILE(GLOB MAIN_FILES ${APP_DIR}/*.cc)
FILE(GLOB TEST_FILES ${TST_DIR}/*.cc)

############## DEPENDANCIES #################
INCLUDE_DIRECTORIES (
  ${INC_DIR}
  #  includes/
)

#LINK_DIRECTORIES ( 
#  includes/
#)

############## COMPILE #################
ADD_LIBRARY(${MYLIB} STATIC ${SRC_FILES})

FOREACH(APPNAME ${MAIN_FILES})
	GET_FILENAME_COMPONENT(BASENAME ${APPNAME} NAME_WE)
	
  if ( ${BASENAME} STREQUAL "cgalConvexHull" )
    if ( CGAL_FOUND )
      #message( STATUS "Compiling: ${BASENAME}" )
      ADD_EXECUTABLE(${BASENAME} ${APPNAME})
      TARGET_LINK_LIBRARIES( ${BASENAME} ${MYLIB}) # ${VTK_LIBRARIES}
    else()
      message(STATUS "Skipping cgalConvexHull.cc compilation; requires the CGAL library (not found).")
    endif()
  else()
    ADD_EXECUTABLE(${BASENAME} ${APPNAME})
    TARGET_LINK_LIBRARIES( ${BASENAME} ${MYLIB}) # ${VTK_LIBRARIES}
  endif()
	    
ENDFOREACH(APPNAME ${MAIN_FILES})

################################
# Unit Tests
################################
# Add test cc file
add_executable( runUnitTests ${SRC_DIR}/tests/DeadSpace_unittest.cc )
# Link test executable against gtest & gtest_main
target_link_libraries(runUnitTests gtest gtest_main ${MYLIB})
add_test( runUnitTests runUnitTests )

FOREACH(TSTNAME ${TEST_FILES})
	GET_FILENAME_COMPONENT(BASENAME ${TSTNAME} NAME_WE)
    ADD_EXECUTABLE(${BASENAME} ${TSTNAME})
    TARGET_LINK_LIBRARIES( ${BASENAME} gtest gtest_main ${MYLIB} )
    add_test( runUnitTests ${BASENAME} )    
ENDFOREACH(TSTNAME ${TEST_FILES})
