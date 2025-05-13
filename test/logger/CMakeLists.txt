# 设置最小 CMake 版本
cmake_minimum_required(VERSION 3.10)

# 设置项目名
project(TestLogger)

# 使用 C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件路径（logger.h 所在目录）
include_directories(../../src/include)

# 添加源文件
set(LOGGER_SRC ../../src/logger.cc)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
# 定义可执行文件
add_executable(testlog testlog.cpp ${LOGGER_SRC})
