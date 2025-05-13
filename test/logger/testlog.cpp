#include "../../src/include/logger.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

using namespace std;

int main() {
    // 设置全局日志级别
    Logger::GetInstance().SetGlobalLevel(DEBUG);
    
    // 基本日志
    LOG_DEBUG("这是一条调试日志");
    LOG_INFO("这是一条信息日志");
    LOG_WARN("这是一条警告日志");
    LOG_ERROR("这是一条错误日志");
    LOG_FATAL("这是一条致命错误日志");
    
    // 带格式化的日志
    LOG_INFO("当前程序已运行 %d 毫秒", 1000);
    
    // 带文件名和行号的日志
    LOG_ERROR_FL("在此处发生错误，错误代码: %d", 404);
    
    // 模拟一些操作
    for (int i = 0; i < 5; ++i) {
        LOG_INFO("循环中... 第 %d 次", i + 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 自定义输出回调示例
    Logger::GetInstance().SetOutputCallback([](const std::string& msg) {
        // 同时输出到控制台和文件
        std::cout << msg << std::endl;
        
        // 还可以在这里添加其他输出方式，如网络、数据库等
        
        // 原文件输出保留
        std::ofstream out_file("custom-log.txt", std::ios::app);
        if (out_file.is_open()) {
            out_file << msg << std::endl;
            out_file.close();
        }
    });
    
    LOG_INFO("使用自定义输出回调后的日志");
    
    // 正常退出前关闭日志系统
    Logger::GetInstance().Shutdown();
    
    return 0;
}