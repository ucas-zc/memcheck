#!/bin/bash

# CMAKE_BUILD_TYPE即编译类型
# CMAKE_BUILD_RPATH即链接库路径
# DEBUG即debug版本，RELEASE即release版本，GTEST采用gtest进行单元测试
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_BUILD_RPATH=/opt/ide ..
make
