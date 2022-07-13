cmake_minimum_required(VERSION 3.10)

project(Combine)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories("${PROJECT_SOURCE_DIR}/include")
aux_source_directory("${PROJECT_SOURCE_DIR}/src" combine_src)
add_executable(combine ${combine_src})

target_link_libraries(combine pthread jsoncpp)