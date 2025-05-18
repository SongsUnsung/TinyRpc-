# 使用教程

## Protobuf 安装
libprotoc 3.21.12
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


## ZooKeeper使用

tar -xzf zookeeper-3.4.10.tar.gz
cd zookeeper-3.4.10

cd conf/

cp zoo_sample.cfg zoo.cfg

sudo vim zoo.cfg

修改dataDir=/tmp/zookeeper为自定义路径
例：dataDir=/home/saul/Downloads/zookeeper-3.4.10/data

cd ..回到项目更目录
cd bin


./zkServer.sh start
ps -ef|grep zookeeper


sudo apt install openjdk-8-jdk -y

./zkCli.sh
启动成功即可


zk的原生开发API（c/c++接口） 
进入上面解压目录src/c下面，zookeeper已经提供了原生的C/C++和Java API开发接口，需要通过源码
编译生成，过程如下：
zookeeper-3.4.10/src/c$ sudo ./configure 
zookeeper-3.4.10/src/c$ sudo make 
zookeeper-3.4.10/src/c$ sudo make install