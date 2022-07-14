cmake_minimum_required(VERSION 3.10)

project(PoseDetect)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
# add_definitions(-std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0)

set(NUITRACK_INCLUDE_PATH ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_LIST_DIR}/include/middleware)
set(NUITRACK_LIBRARY_PATH ${CMAKE_CURRENT_LIST_DIR}/lib) 

include_directories(${NUITRACK_INCLUDE_PATH} "C:/Program Files (x86)/Eigen3/include")
link_directories(${NUITRACK_LIBRARY_PATH})

aux_source_directory("${PROJECT_SOURCE_DIR}/src" pose_src)
add_executable(pose_detect ${pose_src})

set(LIBS nuitrack middleware)

target_link_libraries(pose_detect ${LIBS} jsoncpp)