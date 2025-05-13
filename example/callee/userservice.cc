#include<iostream>
#include<string>
#include "user.pb.h"
#include "mprpcapplication.h" 
#include "rpcprovider.h"


class UserService:public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name,std::string pwd)
    {
        std::cout<<"doing local service: login"<<std::endl;
        std::cout<<"name:"<<name<<"pwd:"<<pwd<<std::endl;
        return true;
    }

    bool Register(uint32_t id,std::string name,std::string pwd)
    {
        std::cout<<"doing local service: Register"<<std::endl;
        std::cout<<"id:"<<id<<"name:"<<name<<"pwd:"<<pwd<<std::endl;
        return true;
    }


    //重写UserServiceRpc的虚函数
    //1.caller==>login(loginRequest)->muduo->callee
    //callee->login(loginRequest)->此方法
    void Login(::google::protobuf::RpcController* controller,
                        const ::fixbug::LoginRequest* request,
                        ::fixbug::LoginResponse* response,
                        ::google::protobuf::Closure* done)
    {
        //框架业务上报请求参数LoginRequest,应用获取相应数据本地业务
        std::string name=request->name();
        std::string pwd=request->pwd();
        //做本地业务
        bool login_result=Login(name,pwd);
        //把响应写入
        fixbug::ResultCode *code=response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");

        response->set_sucess(login_result);
        //执行回调操作，执行响应数据的序列化和网络发送（框架完成）
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                              const ::fixbug::RegisterRequest* request,
                              ::fixbug::RegisterResponse* response,
                              ::google::protobuf::Closure* done)
    {
        uint32_t id=request->id();
        std::string name=request->name();
        std::string pwd=request->pwd();

        bool register_result=Register(id,name,pwd);

        //把响应写入
        fixbug::ResultCode *code=response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(register_result);
        
        //执行回调操作，执行响应数据的序列化和网络发送（框架完成）
        done->Run();
    }

};

int main(int argc,char **argv)
{
    //调用框架初始化操作
    MprpcApplication::Init(argc,argv);

    //provider是一个rpc网络服务对象，把UserServerice对象发布
    RpcProvider provider;
    provider.NotifyService(new UserService());

    provider.Run(); 

    return 0;
}