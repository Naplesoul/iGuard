cmake_minimum_required(VERSION 3.10)

project(PoseDetect)

add_executable(pose_detect main.cpp detector.cpp)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
find_package(OpenPose REQUIRED)
find_package(Boost REQUIRED)

target_link_libraries(pose_detect ${OpenCV_LIBS} ${OpenPose_LIBS} ${Boost_LIBS} gflags)