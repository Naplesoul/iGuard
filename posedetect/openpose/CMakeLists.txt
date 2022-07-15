cmake_minimum_required(VERSION 3.10)

project(PoseDetect)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories("${PROJECT_SOURCE_DIR}/include")
aux_source_directory("${PROJECT_SOURCE_DIR}/src" pose_src)
add_executable(pose ${pose_src})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
find_package(OpenPose REQUIRED)
find_package(Boost REQUIRED)

target_link_libraries(pose ${OpenCV_LIBS} ${OpenPose_LIBS} ${Boost_LIBS} gflags realsense2 jsoncpp)