#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>  // 用于 getopt 函数解析命令行参数

// 静态成员变量定义：配置对象实例（全局唯一）
MprpcConfig MprpcApplication::m_config;

// 显示程序启动参数的帮助信息
void showArgsHelp()
{
    std::cout << "format: command -i <configfile>" << std::endl;
}

// 初始化函数：整个 MPRPC 框架的入口初始化
void MprpcApplication::Init(int argc, char **argv)
{
    // 如果参数数量小于2（即未指定任何选项），打印帮助并退出
    if (argc < 2)
    {
        showArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;  // 用于 getopt 的返回值
    std::string config_file;  // 保存传入的配置文件路径

    // 使用 getopt 解析命令行参数
    // 语法：command -i configfile
    // "i:" 表示 -i 后必须跟一个参数
    while ((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':  // 解析到 -i 参数
            config_file = optarg;  // 获取 -i 后跟的参数，即配置文件路径
            break;
        case '?':  // 参数非法，例如输入了未知选项
            showArgsHelp();
            exit(EXIT_FAILURE);
        case ':':  // 缺少选项参数，例如输入 -i 却没有跟路径
            showArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // 使用 config_file 来加载配置文件
    m_config.LoadConfigFile(config_file.c_str());

    // 可选调试信息：输出读取到的配置信息
    /*
    std::cout << "rpcserverip: " << m_config.Load("rpcserverip") << std::endl;
    std::cout << "rpcserverport: " << m_config.Load("rpcserverport") << std::endl;
    std::cout << "zookeeperip: " << m_config.Load("zookeeperip") << std::endl;
    std::cout << "zookeeperport: " << m_config.Load("zookeeperport") << std::endl;
    */
}

// 获取全局唯一的 MprpcApplication 实例
// 单例模式的关键函数，程序中任何地方都可以通过此函数访问唯一实例
MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;  // 局部静态变量，只初始化一次
    return app;
}

// 提供对配置对象的全局访问接口
MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}
