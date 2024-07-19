cmake_minimum_required(VERSION 3.16)
project(MCHLG)

# only print error
if(MSVC)
  add_compile_options(/W0)
else()
  add_compile_options(-w)
endif()

# set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

# multi-thread
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()


set(DEFLIB_HOME
  ${PROJECT_SOURCE_DIR}/module/def/5.8-p027
)
set(LEFLIB_HOME
  ${PROJECT_SOURCE_DIR}/module/lef/5.8-p027
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(GD REQUIRED)

add_subdirectory(${DEFLIB_HOME})
add_subdirectory(${LEFLIB_HOME})

set(MCHLG_SRC
    src/utils.cpp
    src/rtree.cpp
    src/matrix.cpp
    src/munkres.cpp

    src/main.cpp
    src/parser.cpp
    src/parser_helper.cpp

    src/nall.cpp
    src/pre.cpp
    src/multi_Imp.cpp
    src/ripup.cpp
    src/post.cpp

    utils/log.cpp
    utils/draw.cpp
)

############################################################
# Include directories
############################################################

include_directories (
  ${LEFLIB_HOME}/lef
  ${LEFLIB_HOME}/lefzlib
  ${LEFLIB_HOME}/clef
  ${LEFLIB_HOME}/clefzlib

  ${DEFLIB_HOME}/def
  ${DEFLIB_HOME}/defzlib
  ${DEFLIB_HOME}/cdef
  ${DEFLIB_HOME}/cdefzlib

  include
  utils
  ${GD_INCLUDE_DIR}
)

############################################################
# Libraries setting
############################################################

add_library( MCHLGlib STATIC
  ${MCHLG_SRC}
)

set_target_properties( MCHLGlib
  PROPERTIES OUTPUT_NAME "MCHLGlib"
)

############################################################
# Executable setting
############################################################

add_executable( MCHLG
  ${MCHLG_SRC}
)

target_link_libraries( MCHLG
  PUBLIC
  def
  defzlib
  cdef
  cdefzlib
  lef
  lefzlib
  clef
  clefzlib
  ${GD_LIBRARIES}
)



add_dependencies( MCHLG def )
add_dependencies( MCHLG lef )

add_dependencies( MCHLGlib def )
add_dependencies( MCHLGlib lef )