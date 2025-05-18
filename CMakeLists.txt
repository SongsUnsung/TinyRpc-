cmake_minimum_required(VERSION 3.0)
project(mprpc)

set(CMAKE_BUILD_TYPE "Debug")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

#link_libraries(${PROJECT_SOURCE_DIR}/lib)


# 发现 Protobuf 和 Abseil
find_package(PkgConfig REQUIRED)
pkg_check_modules(PROTOBUF REQUIRED protobuf)
pkg_check_modules(ABSL REQUIRED absl_log_internal_check_op absl_log_internal_message absl_strings absl_base)

# 把路径传给子目录
set(THIRD_PARTY_INCLUDE_DIRS ${PROTOBUF_INCLUDE_DIRS} ${ABSL_INCLUDE_DIRS})
set(THIRD_PARTY_LIBRARY_DIRS ${PROTOBUF_LIBRARY_DIRS} ${ABSL_LIBRARY_DIRS})
set(THIRD_PARTY_LIBS ${PROTOBUF_LIBRARIES} ${ABSL_LIBRARIES})


add_subdirectory(src)
add_subdirectory(example)
