# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.12.0)

# 链接库路径
if(CMAKE_BUILD_RPATH)
    message("CMAKE_BUILD_RPATH: ${CMAKE_BUILD_RPATH}")
endif()

# 项目信息
project (memcheck)
set (MEMCHECK_VERSION 1)
set (MEMCHECK_VERSION 0)
set (MEMCHECK_VERSION 0)

# 设置编译选项
# debug调试版本
if (CMAKE_BUILD_RPATH AND (CMAKE_BUILD_TYPE MATCHES DEBUG))
    add_definitions(-D_DEBUG_)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -g --std=c++11")
endif()

# 目录下的所有源文件
aux_source_directory(src SRC_FILES)

# 头文件目录
include_directories(${PROJECT_SOURCE_DIR}/hdr)

# 库文件目录
include_directories(${CMAKE_BUILD_RPATH}/include)
link_directories(${CMAKE_BUILD_RPATH}/lib)

# 指定生成目标
if (CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_executable(memcheck ${SRC_FILES})
endif()
