# 使用教程

## Protobuf 安装

github源代码下载地址：https://github.com/google/protobuf 

### 解压压缩包：unzip protobuf-master.zip 


### 安装所需工具：
sudo apt-get install autoconf automake libtool curl make g++ unzip 

### 安装abseil
下载源码（如果 GitHub 访问困难，可用镜像源 https://hub.nuaa.cf/abseil/abseil-cpp）

git clone https://github.com/abseil/abseil-cpp.git

cd abseil-cpp

mkdir build && cd build

cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_POSITION_INDEPENDENT_CODE=ON

make -j$(nproc)
sudo make install
更新动态库缓存
sudo ldconfig


### 进入protobuf-main目录 
cd protobuf-main

mkdir build&&cd build

### 生成makefile文件
cmake .. \
  -Dprotobuf_BUILD_TESTS=OFF \
  -Dabsl_DIR=/usr/local/lib/cmake/absl

### 编译源代码
make -j$(nproc)

### 安装probuf
sudo make install 

### 刷新动态库
sudo ldconfig

