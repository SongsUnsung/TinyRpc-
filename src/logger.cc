// logger.cpp
#include "logger.h"
#include <time.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

// 默认日志级别为INFO
Logger::Logger() : m_globalLevel(INFO), m_running(true)
{
    // 初始化输出回调为文件输出
    m_outputCallback = [this](const std::string& msg) {
        // 获取当前日期作为文件名
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        tm now_tm;
        
#ifdef _WIN32
        localtime_s(&now_tm, &now_time_t);
#else
        localtime_r(&now_time_t, &now_tm);
#endif
        
        char file_name[128];
        sprintf(file_name, "%d-%02d-%02d-log.txt",
                now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
        
        // 使用C++文件流，更安全可靠
        std::ofstream out_file(file_name, std::ios::app);
        if (!out_file.is_open()) {
            std::cerr << "Failed to open log file: " << file_name << std::endl;
            return;
        }
        
        out_file << msg << std::endl;
        out_file.close();
    };
    
    // 启动日志线程
    m_logThread = std::make_shared<std::thread>(&Logger::WriteLogTask, this);
}

Logger& Logger::GetInstance()
{
    // C++11 保证静态局部变量的线程安全初始化
    static Logger logger;
    return logger;
}

void Logger::SetOutputCallback(std::function<void(const std::string&)> cb)
{
    if (cb) {
        m_outputCallback = std::move(cb);
    }
}

void Logger::SetGlobalLevel(LogLevel level)
{
    m_globalLevel = level;
}

bool Logger::IsLevelEnabled(LogLevel level) const
{
    return level >= m_globalLevel;
}

void Logger::Log(LogLevel level, const std::string& msg)
{
    if (!IsLevelEnabled(level)) {
        return;
    }
    
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    tm now_tm;
#ifdef _WIN32
    localtime_s(&now_tm, &now_time_t);
#else
    localtime_r(&now_time_t, &now_tm);
#endif
    
    // 格式化时间和日志级别
    std::ostringstream oss;
    oss << std::setfill('0') 
        << std::setw(4) << (now_tm.tm_year + 1900) << '-'
        << std::setw(2) << (now_tm.tm_mon + 1) << '-'
        << std::setw(2) << now_tm.tm_mday << ' '
        << std::setw(2) << now_tm.tm_hour << ':'
        << std::setw(2) << now_tm.tm_min << ':'
        << std::setw(2) << now_tm.tm_sec << '.'
        << std::setw(3) << now_ms.count()
        << " [" << LogLevelToString(level) << "] "
        << msg;
    
    // 将完整日志消息加入队列
    m_lckQue.Push(oss.str());
}

void Logger::WriteLogTask()
{
    while (m_running) {
        try {
            std::string msg = m_lckQue.Pop();
            if (m_outputCallback) {
                m_outputCallback(msg);
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in log thread: " << e.what() << std::endl;
        }
    }
    
    // 处理关闭时队列中剩余的日志
    while (!m_lckQue.Empty()) {
        std::string msg = m_lckQue.Pop();
        if (m_outputCallback) {
            m_outputCallback(msg);
        }
    }
}

const char* Logger::LogLevelToString(LogLevel level)
{
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO";
        case WARN:  return "WARN";
        case ERROR: return "ERROR";
        case FATAL: return "FATAL";
        default:    return "UNKNOWN";
    }
}

void Logger::Shutdown()
{
    // 标记退出并等待日志线程结束
    m_running = false;
    m_lckQue.Notify(); // 唤醒等待中的线程
    
    if (m_logThread && m_logThread->joinable()) {
        m_logThread->join();
        m_logThread.reset();
    }
}