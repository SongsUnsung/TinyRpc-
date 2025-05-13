#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <functional> 
#include <string>
#include <google/protobuf/descriptor.h> 
#include <unordered_map> 

// 框架提供专门服务发布rpc服务的网络对象类
class RpcProvider
{
public:
    // 该接口用于外部使用，向框架注册并发布 RPC 服务方法
    // 通过传入的 service 对象将服务和它的 RPC 方法注册到框架中
    void NotifyService(google::protobuf::Service *service);

    // 启动 RPC 服务节点，开始提供 RPC 调用服务
    // 设置 TCP 服务器，监听指定端口和 IP 地址，等待客户端连接请求
    void Run();

private:
    // 用于框架事件循环的对象，处理 TCP 连接和通信
    muduo::net::EventLoop m_eventLoop;

    // ServiceInfo 结构体，用于存储每个服务的信息
    // 包括服务对象和该服务下所有方法的描述信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;  // 服务对象
        // 存储该服务下的所有方法的描述信息（方法名与方法描述符的映射）
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };

    // 存储服务对象的哈希表，以服务名称为键
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // 连接回调函数，当客户端连接时调用
    void OnConnection(const muduo::net::TcpConnectionPtr&);

    // 消息回调函数，当收到客户端消息时调用
    void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);

    //closure回调操作，用于序列化rpc响应和网络发送
    void SendRpcRespond(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);
};
