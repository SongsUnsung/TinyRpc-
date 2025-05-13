cmake_minimum_required(VERSION 3.10)
project(MyProtobufProject)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 pkg-config
find_package(PkgConfig REQUIRED)

# 查找 Protobuf 和 Abseil 库
pkg_check_modules(PROTOBUF REQUIRED protobuf)
pkg_check_modules(ABSL REQUIRED absl_log_internal_check_op absl_log_internal_message absl_strings absl_base)

# 添加可执行文件
add_executable(my_program main.cc test.pb.cc)

# 设置头文件和库路径
target_include_directories(my_program PRIVATE ${PROTOBUF_INCLUDE_DIRS} ${ABSL_INCLUDE_DIRS})
target_link_directories(my_program PRIVATE ${PROTOBUF_LIBRARY_DIRS} ${ABSL_LIBRARY_DIRS})
target_link_libraries(my_program PRIVATE ${PROTOBUF_LIBRARIES} ${ABSL_LIBRARIES})