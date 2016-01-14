cmake_minimum_required(VERSION 2.6)

project(myInstantIOPlugin)

set(DTK_ROOT "/usr/local" CACHE PATH "Path to diverse toolkit")
set(IR_ROOT "/opt/instantReality" CACHE PATH "Path to InstantReality")

set(DTK_HEADERS "${DTK_ROOT}/include")
set(DTK_LIBS "${DTK_ROOT}/lib")

set(IR_HEADERS "${IR_ROOT}/include")
set(IR_LIBS "${IR_ROOT}/lib")

set(MOD "dtkShmMove")

# adds the InstantIO lib and include dir to search paths
include_directories(${IR_HEADERS} ${DTK_HEADERS})
link_directories(${IR_LIBS} ${DTK_LIBS})

# adds the source files to the project
set(src ${MOD}.cpp)

# create a dynamic library with .iio ending
add_library(${MOD} SHARED ${src})
set_target_properties(${MOD} PROPERTIES SUFFIX .iio PREFIX "")
target_link_libraries(${MOD} InstantIO dtk)

# install the created lib to the InstantIO lib directory
#install(TARGETS ${MOD} DESTINATION ${IR_BIN})
