cmake_minimum_required(VERSION 3.10)

project(Calibrate)

find_package(OpenCV REQUIRED)
find_package(freenect2 REQUIRED)

INCLUDE_DIRECTORIES(${freenect2_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

add_executable(calibrate main.cpp)

target_link_libraries(calibrate jsoncpp ${freenect2_LIBRARIES} ${OpenCV_LIBS})