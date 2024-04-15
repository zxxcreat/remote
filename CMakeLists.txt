
cmake_minimum_required(VERSION 3.10)
project(aa)
#SET(CMAKE_BUILD_TYPE "Release")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
SET(CMAKE_BUILD_TYPE "Debug")
# 全局包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/head)
include_directories(${PROJECT_SOURCE_DIR}/head/pb)
# 添加子目录
add_subdirectory(pbout)
#add_subdirectory(gateway)
#add_subdirectory(center)
#add_subdirectory(login)

