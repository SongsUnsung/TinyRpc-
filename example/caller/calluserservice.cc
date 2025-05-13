#include<iostream>
#include"mprpcapplication.h"
#include"user.pb.h"


int main(int argc,char** argv)
{
    MprpcApplication::Init(argc,argv);

    //login调用

    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    fixbug::LoginRequest request;

    request.set_name("zhang san");
    request.set_pwd("123456");

    fixbug::LoginResponse response;

    stub.Login(nullptr,&request,&response,nullptr);
    
    if(0==response.result().errcode())
    {
        std::cout<<"rpc login response:"<<response.sucess()<<std::endl;
    }
    else
    {
        std::cout<<"rpc login response error:"<<response.result().errmsg()<<std::endl;
    }

    fixbug::RegisterRequest req;
    req.set_id(0);
    req.set_name("mprpc");
    req.set_pwd("666666");
    fixbug::RegisterResponse rsp;

    //同步发送
    stub.Register(nullptr,&req,&rsp,nullptr);

    if(0==rsp.result().errcode())
    {
        std::cout<<"rpc register response:"<<rsp.sucess()<<std::endl;
    }
    else
    {
        std::cout<<"rpc register response error:"<<rsp.result().errmsg()<<std::endl;
    }



    return 0;
}