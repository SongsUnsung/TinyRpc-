#include "mprpcchannel.h"
#include<string>
#include "rpcheader.pb.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<mprpcconfig.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<mprpcapplication.h>
#include <unistd.h>
#include "mprpccontroller.h"

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller,
                        const google::protobuf::Message* request,
                        google::protobuf::Message* response,
                        google::protobuf::Closure* done)
{
    // 获取服务名称与方法名称（如 LoginService.Login）
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();       // 服务名
    std::string method_name = method->name();     // 方法名

    // 序列化请求参数为字符串
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str)) 
    {
        args_size = args_str.size();  // 获取参数大小
    } 
    else 
    {
        controller->SetFailed("serialize request error!");
        return;
    }

    // 构造自定义的 RPC 头部（包含服务名、方法名、参数大小）
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    // 序列化 RPC 头部
    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str)) 
    {
        header_size = rpc_header_str.size();  // 头部大小
    } 
    else 
    {
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    // 构造最终发送的字符串结构：
    // [4字节 header_size][header_str][args_str]
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));  // 插入前4字节的 header_size
    send_rpc_str += rpc_header_str;                               // 添加头部数据
    send_rpc_str += args_str;                                     // 添加参数数据

    // 打印调试信息
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "debug:" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    // 创建 TCP 客户端 socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1) 
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "create socket error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 从配置中读取 rpc 服务器 IP 和端口
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    // 配置服务器地址结构
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);              // 主机字节序转网络字节序
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 发起连接请求
    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr))) 
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "connect error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 发送请求数据
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0)) 
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 接收返回结果（最多接收 1024 字节）
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0))) 
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 将收到的数据反序列化为 response 对象
    if (!response->ParseFromArray(recv_buf, recv_size)) 
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "parse error! errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 关闭 socket 连接
    close(clientfd);
}
