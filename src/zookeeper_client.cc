#include "zookeeper_client.h"             
#include "mprpc_application.h"         
#include <semaphore.h>                
#include <iostream> 
#include "logger.h"


// ZooKeeper 全局 Watcher 回调函数
void global_watcher(zhandle_t *zh, int type,
                    int state, const char *path, void *watcherCtx)
{
    // 如果是会话事件
    if (type == ZOO_SESSION_EVENT)
    {
        // 如果连接状态为已连接
        if (state == ZOO_CONNECTED_STATE)
        {
            // 获取上下文中保存的信号量指针
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            // 释放信号量，通知主线程连接完成
            sem_post(sem);
        }
        else if (state == ZOO_EXPIRED_SESSION_STATE) 
        {
            LOG_FATAL_FL("ZooKeeper会话已过期");
        }
    }
}

// 构造函数：初始化句柄为空
ZkClient::ZkClient() : m_zhandle(nullptr)
{
    LOG_DEBUG("ZooKeeper客户端初始化");
}

// 析构函数：如果句柄不为空，关闭 ZooKeeper 连接
ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        LOG_DEBUG("关闭ZooKeeper连接");
        zookeeper_close(m_zhandle); // 关闭连接释放资源
    }
}

// 启动 ZooKeeper 客户端
void ZkClient::Start()
{
    // 1. 从配置中读取 zookeeper 的 IP 和端口
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port; // 拼接连接字符串

    LOG_INFO("正在连接ZooKeeper服务器 [%s]", connstr.c_str());

    // 2. 初始化 ZooKeeper 客户端连接，注册全局 Watcher 回调
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle)
    {
        //std::cout << "zookeeper_init error!" << std::endl;
        LOG_FATAL_FL("ZooKeeper客户端初始化失败");
        exit(EXIT_FAILURE); // 初始化失败退出程序
    }

    // 3. 使用信号量等待连接建立
    sem_t sem;
    sem_init(&sem, 0, 0); // 初始化信号量，初始值为0
    zoo_set_context(m_zhandle, &sem); // 将信号量设置为上下文，供 watcher 使用

     LOG_DEBUG("等待ZooKeeper连接建立...");

    sem_wait(&sem); // 等待连接建立（在 global_watcher 中 sem_post）
    LOG_INFO("ZooKeeper连接已建立");
    std::cout << "zookeeper_init success!" << std::endl;
}

// 创建 znode 节点（临时或永久）
void ZkClient::Create(const char *path, const char *data, int datalen, int state /*=0*/)
{
    char path_buffer[128]; // 创建成功后返回的实际路径缓冲区
    int bufferlen = sizeof(path_buffer);

    // 1. 检查该节点是否存在
    int flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag) // 如果节点不存在，则创建
    {
        // 2. 创建节点（支持临时或永久，取决于 state 参数）
        flag = zoo_create(m_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if (flag == ZOK)
        {
            LOG_INFO("节点创建成功 [%s]", path);
            std::cout << "znode create success... path:" << path << std::endl;
        }
        else
        {
            LOG_ERROR_FL("节点创建失败 [%s] 错误码: %d 错误描述: %s",
                       path, flag, zerror(flag));
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error ... path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    // 如果节点已存在，不做处理（根据需要也可选择覆盖或更新数据）
    else if (flag == ZOK)
    {
        LOG_WARN("节点已存在 [%s]", path);
    }
    else
    {
        LOG_ERROR_FL("节点检查失败 [%s] 错误码: %d", path, flag);
    }
}

// 获取指定路径节点的内容
std::string ZkClient::GetData(const char *path)
{
    char buffer[64]; // 用于存放节点数据
    int bufferlen = sizeof(buffer);

    // 1. 获取 znode 节点的数据内容
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if (flag != ZOK)
    {
        LOG_ERROR_FL("获取节点数据失败 [%s] 错误码: %d", path, flag);
        std::cout << "get znode error... path:" << path << std::endl;
        return ""; // 失败返回空字符串
    }
    else
    {
        LOG_DEBUG("成功获取节点数据 [%s] 数据大小: %d", path, bufferlen);
        return buffer; // 成功返回数据
    }
}