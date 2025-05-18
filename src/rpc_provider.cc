#include "rpc_provider.h"                      
#include "mprpc_application.h"
#include "rpc_header.pb.h"                
#include <iostream>    
#include "zookeeper_client.h"
#include "logger.h"

// 用于框架内部发布服务：将一个服务对象（继承自 protobuf 的 Service）注册进来
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    // 获取服务的描述信息：包括服务名称、方法等
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();

    // 获取服务的名称（是 string_view 类型，需要转成 std::string）
    std::string service_name = std::string(pserviceDesc->name());

    //std::cout<<"service_name"<<service_name<<std::endl;
    LOG_INFO("[服务注册] 发现服务: %s", service_name.c_str());


    // 获取该服务包含的方法数量
    int methodCnt = pserviceDesc->method_count();

    // 遍历服务中所有的方法
    for (int i = 0; i < methodCnt; ++i)
    {
        // 获取第 i 个方法的描述符对象（包含方法名、参数、返回值等信息）
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);

        // 获取方法名称（如 Login、Register 等），同样转为 std::string
        std::string method_name = std::string(pmethodDesc->name());

        // 将方法名与方法描述符指针存入 service_info 的方法映射表中
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        std::cout<<"method_name:"<<method_name<<std::endl;

    }

    // 将该服务对象本身保存起来
    service_info.m_service = service;

    // 将服务名称与其对应的信息结构体存入全局服务表 m_serviceMap 中
    // 后续收到 RPC 请求时，可通过服务名和方法名找到对应处理函数
    m_serviceMap.insert({service_name, service_info});
}
    
// 启动 RPC 服务节点，提供 RPC 方法调用服务（相当于启动 RPC 服务器）
void RpcProvider::Run()
{
    // 从配置文件读取服务监听的 IP 地址
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");

    // 从配置文件读取监听的端口号，并转换为 uint16_t 类型
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    LOG_INFO("初始化服务端点 [%s:%d]", ip.c_str(), port);

    // 创建一个 TCP 地址对象（Muduo 提供）
    muduo::net::InetAddress address(ip, port);

    // 创建一个 TCP 服务器对象，传入 event loop、绑定地址和服务器名称
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    // 设置连接建立/断开的回调函数（通过 bind 绑定类的成员函数）
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));

    // 设置消息到来时的回调函数（数据读取处理）
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3));

    // 设置工作线程数量（多线程处理 RPC 请求）
    server.setThreadNum(4);
    LOG_DEBUG("工作线程数: %d", 4);

    ZkClient zkCli;
    zkCli.Start();
    //service_name为永久性节点 method_name为临时性节点
    for(auto &sp:m_serviceMap)
    {
        std::string service_path="/"+sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0);

        for(auto &mp:sp.second.m_methodMap)
        {
            //service_name/method_name  /UserServiceRpc/Login 
            std::string method_path=service_path+"/"+mp.first;
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            //ZOO_EPHEMERAL表示znode是临时性节点
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }


    // 启动 RPC 服务的信息打印
    std::cout << "RpcProvider start service at ip: " << ip << " port: " << port << std::endl;
    LOG_INFO("RPC服务启动 [%s:%d]", ip.c_str(), port);
    // 启动服务器并进入事件循环，开始监听和处理连接
    server.start();
    m_eventLoop.loop();
}

// 新连接建立/断开时的回调函数
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    // 如果连接断开，关闭连接
    if (!conn->connected())
    {
        LOG_DEBUG("连接关闭 [%s]", conn->peerAddress().toIpPort().c_str());
        conn->shutdown();  // 底层会自动释放资源
    }
}


//框架内部，rpcprovider和prcconsumer协商好通信用的protobuf数据类型
//service_name method_name args 定义proto的message的类型，解析数据头的序列化和反序列化
// 有消息到达时的回调函数（请求数据在 Buffer 中）

void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp)
{
    //网络上接受rpc调用请求的字符流
    std::string recv_buf=buffer->retrieveAllAsString();

    //从字符流中读取前4个字节的内容
    uint32_t header_size=0;
    //recv_buf.copy((char*)&header_size,4,0);
    memcpy(&header_size, recv_buf.data(), 4);

    //根据header_size读取数据头的原始字符流
    std::string rpc_header_str=recv_buf.substr(4,header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size=0;
    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name=rpcHeader.service_name();
        method_name=rpcHeader.method_name();
        args_size=rpcHeader.args_size();
    }
    else
    {
        //数据头反序列化失败
        std::cout<<"rpc_header_str:"<<rpc_header_str<<"parse error!"<<std::endl;
        return;
    }   
   //获取rpc方法参数的字符流数据
    std::string args_str=recv_buf.substr(4+header_size,args_size);


  /*   //打印调试信息
    std::cout<<"----------------------------------------"<<std::endl;
    std::cout<<"Provider_debug:"<<std::endl;
    std::cout<<"header_size: "<<header_size<<std::endl;
    std::cout<<"rpc_header_str: "<<rpc_header_str<<std::endl;
    std::cout<<"service_name: "<<service_name<<std::endl;
    std::cout<<"method_name: "<<method_name<<std::endl;
    std::cout<<"args_str: "<<args_str<<std::endl;
    std::cout << "args_str hex: ";
    for (unsigned char c : args_str) {
    printf("%02x ", c);
    }
    std::cout << std::endl;
    std::cout<<"----------------------------------------"<<std::endl;
 */
    //获取service对象和method对象

    auto it=m_serviceMap.find(service_name);
    if(it==m_serviceMap.end())
    {
        //std::cout<<service_name<<"is not exist!"<<std::endl;
        LOG_ERROR_FL("未知方法 [%s.%s]", 
                   service_name.c_str(), method_name.c_str());
        return;
    }

    auto mit=it->second.m_methodMap.find(method_name);
    if(mit==it->second.m_methodMap.end())
    {
        LOG_ERROR_FL("请求参数解析失败 [服务: %s 方法: %s]", 
                   service_name.c_str(), method_name.c_str());
        std::cout<<service_name<<":"<<method_name<<"is not exist!"<<std::endl;
        return;
    }

    //获取service
    google::protobuf::Service *service=it->second.m_service;
    //获取method
    const google::protobuf::MethodDescriptor *method=mit->second;

    //生成rpc方法调用的请求request的响应和response参数
    google::protobuf::Message *request=service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str))
    {
        LOG_ERROR_FL("请求参数解析失败 [服务: %s 方法: %s]", 
                   service_name.c_str(), method_name.c_str());
        std::cout<<"request parse error,content:"<<args_str<<std::endl;
        return;
    }
    google::protobuf::Message *response=service->GetResponsePrototype(method).New();

    LOG_INFO("开始处理请求 [%s.%s]", service_name.c_str(), method_name.c_str());

    //绑定closure回调
    google::protobuf::Closure* done =
    google::protobuf::NewCallback<RpcProvider,
                            const muduo::net::TcpConnectionPtr&, 
                            google::protobuf::Message*>(
        this, &RpcProvider::SendRpcRespond, conn, response);


    service->CallMethod(method,nullptr,request,response,done);
}

//closure回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcRespond(const muduo::net::TcpConnectionPtr& conn,google::protobuf::Message* response)
{
    std::string response_str;
    if(response->SerializeToString(&response_str))
    {
        //序列化成功后，通过网络把rpc方法执行结果发送给rpc调用方
        LOG_DEBUG("发送响应 [大小: %zu bytes]", response_str.size());
        conn->send(response_str);
        conn->shutdown();
    }
    else
    {
        LOG_ERROR_FL("响应序列化失败 [类型: %s]", 
                   response->GetTypeName().c_str());
        std::cout<<"serialize respinse_str error!"<<std::endl;
    }
    conn->shutdown();
}
