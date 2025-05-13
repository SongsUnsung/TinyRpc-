// logger.h
#pragma once
#include "lockqueue.h"
#include <string>
#include <memory>
#include <functional>

// 扩展日志级别
enum LogLevel
{
    DEBUG,  // 新增调试级别
    INFO,
    WARN,   // 新增警告级别
    ERROR,  // 修正拼写：ERRO -> ERROR
    FATAL   // 新增致命错误级别
};

class Logger
{
public:
    static Logger& GetInstance();
    
    // 设置日志输出回调函数
    void SetOutputCallback(std::function<void(const std::string&)> cb);
    
    // 设置全局日志级别过滤器
    void SetGlobalLevel(LogLevel level);
    
    // 记录日志（增加了日志级别参数）
    void Log(LogLevel level, const std::string& msg);
    
    // 返回日志级别对应的字符串
    const char* LogLevelToString(LogLevel level);
    
    // 关闭日志系统，确保所有日志都被写入
    void Shutdown();
    
    // 判断某级别日志是否会被记录
    bool IsLevelEnabled(LogLevel level) const;

private:
    LogLevel m_globalLevel;
    LockQueue<std::string> m_lckQue;
    bool m_running;
    std::function<void(const std::string&)> m_outputCallback;
    std::shared_ptr<std::thread> m_logThread;
    
    // 私有构造函数
    Logger();
    
    // 删除拷贝构造和移动构造
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;
    
    // 写日志线程的实际执行函数
    void WriteLogTask();
};

// 改进的宏定义，更简洁、更统一
#define LOG_BASE(level, format, ...) \
    do { \
        if (Logger::GetInstance().IsLevelEnabled(level)) { \
            char buffer[2048] = {0}; \
            snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
            Logger::GetInstance().Log(level, buffer); \
        } \
    } while (0)

#define LOG_DEBUG(format, ...) LOG_BASE(DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) LOG_BASE(INFO, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) LOG_BASE(WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LOG_BASE(ERROR, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) LOG_BASE(FATAL, format, ##__VA_ARGS__)

// 文件名和行号版本的日志宏
#define LOG_DEBUG_FL(format, ...) LOG_BASE(DEBUG, "[%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO_FL(format, ...) LOG_BASE(INFO, "[%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN_FL(format, ...) LOG_BASE(WARN, "[%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR_FL(format, ...) LOG_BASE(ERROR, "[%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_FATAL_FL(format, ...) LOG_BASE(FATAL, "[%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__)