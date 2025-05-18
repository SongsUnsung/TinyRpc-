#pragma once
#include "mprpc_config.h"
#include "mprpc_channel.h"
#include "mprpc_controller.h"

// MprpcApplication 是整个 MPRPC 框架的初始化类（单例模式）。
// 负责框架初始化、读取配置文件，并全局提供配置访问接口。

class MprpcApplication
{
public:
    // 初始化函数，在程序启动时调用
    // 作用：
    // 1. 解析命令行参数（如 -i 配置文件路径）
    // 2. 加载配置文件内容到 m_config 中
    static void Init(int argc, char **argv);

    // 获取全局唯一的 MprpcApplication 实例（单例模式）
    // 保证全局只能存在一个应用实例
    static MprpcApplication& GetInstance();

    // 获取配置对象 MprpcConfig 的引用
    // 用于读取如 rpcserverip、rpcserverport、zookeeperip 等配置项
    static MprpcConfig& GetConfig();

private:
    // 全局唯一的配置对象，用于保存解析后的配置信息
    static MprpcConfig m_config;

    // 私有构造函数，禁止外部实例化
    MprpcApplication() {};

    // 禁止拷贝构造函数，防止拷贝实例（保证单例）
    MprpcApplication(const MprpcApplication&) = delete;

    // 禁止移动构造函数
    MprpcApplication(MprpcApplication&&) = delete;
};
