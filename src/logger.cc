// logger.cpp
#include "logger.h"
#include <time.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

Logger::Logger() : m_globalLevel(INFO), m_running(true) {
    // 初始化默认输出回调
    m_outputCallback = [this](const LogItem& item) {
        // 格式化时间
        auto now_time_t = std::chrono::system_clock::to_time_t(item.timestamp);
        tm now_tm;
#ifdef _WIN32
        localtime_s(&now_tm, &now_time_t);
#else
        localtime_r(&now_time_t, &now_tm);
#endif
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            item.timestamp.time_since_epoch()) % 1000;

        // 格式化成字符串
        std::ostringstream oss;
        oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S")
            << "." << std::setfill('0') << std::setw(3) << ms.count()
            << " [" << LogLevelToString(item.level) << "] "
            << item.message;

        // 文件处理
        std::string new_date = GetCurrentDateString(item.timestamp);
        std::lock_guard<std::mutex> lock(m_fileMutex);
        
        if (new_date != m_currentDate) {
            if (m_currentFile.is_open()) {
                m_currentFile.close();
            }
            m_currentDate = new_date;
            std::string filename = m_currentDate + "-log.txt";
            m_currentFile.open(filename, std::ios::app);
            if (!m_currentFile.is_open()) {
                std::cerr << "Failed to open log file: " << filename << std::endl;
                return;
            }
        }

        if (m_currentFile.is_open()) {
            m_currentFile << oss.str() << std::endl;
            static int counter = 0;
            if (++counter % 100 == 0) {
                m_currentFile.flush();
            }
        }
    };

    m_logThread = std::make_shared<std::thread>(&Logger::WriteLogTask, this);
}

Logger::~Logger() {
    Shutdown(); // 确保线程停止
    std::lock_guard<std::mutex> lock(m_fileMutex);
    if (m_currentFile.is_open()) {
        m_currentFile.close();
    }
}

Logger& Logger::GetInstance()
{
    // C++11 保证静态局部变量的线程安全初始化
    static Logger logger;
    return logger;
}

void Logger::SetOutputCallback(std::function<void(const LogItem&)> cb) {
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
    if (!IsLevelEnabled(level)) return;
    if (!m_running.load()) return;
    
    m_lckQue.Push(LogItem{
        std::chrono::system_clock::now(),
        level,
        std::move(msg)
    });
}

void Logger::WriteLogTask()
{
    std::vector<LogItem> bulk;
    bulk.reserve(100);

     while (m_running) {
        try {
            if (m_lckQue.PopBulk(bulk, 100, 100)) {
                for (auto& item : bulk) {
                    if (m_outputCallback) {
                        m_outputCallback(item);
                    }
                }
                bulk.clear();
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in log thread: " << e.what() << std::endl;
        }
    }
    
    // 处理关闭时队列中剩余的日志
    while (!m_lckQue.Empty()) {
        auto item = m_lckQue.Pop();
        if (m_outputCallback) {
            m_outputCallback(item);
        }
    }
}

std::string Logger::GetCurrentDateString(
    std::chrono::system_clock::time_point tp) const 
{
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(tp);
    tm now_tm;
#ifdef _WIN32
    localtime_s(&now_tm, &now_time_t);
#else
    localtime_r(&now_time_t, &now_tm);
#endif
    
    std::ostringstream oss;
    oss << std::setfill('0')
        << (now_tm.tm_year + 1900)
        << std::setw(2) << (now_tm.tm_mon + 1)
        << std::setw(2) << now_tm.tm_mday;
    return oss.str();
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
     bool expected = true;
    if (!m_running.compare_exchange_strong(expected, false)) return;
    
    m_lckQue.SetExit();
    
    if (m_logThread && m_logThread->joinable()) {
        m_logThread->join();
        m_logThread.reset();
    }

    // 处理剩余日志
    std::vector<LogItem> bulk;
    while (m_lckQue.PopBulk(bulk, 100, 0)) {
        for (auto& item : bulk) {
            m_outputCallback(item);
        }
        bulk.clear();
    }

    std::lock_guard<std::mutex> lock(m_fileMutex);
    if (m_currentFile.is_open()) {
        m_currentFile.close();
    }
}