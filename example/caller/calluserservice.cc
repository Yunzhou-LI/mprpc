#include<iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"

int main(int argc,char** argv)
{
    //整个程序启动以后，想使用rpc框架来享受rpc服务，一定要先调用框架初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);

    //演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    //rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    //rpc方法的响应参数
    fixbug::LoginResponse response;

    stub.Login(nullptr,&request,&response,nullptr);   //RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送
    //调用完成，取结果
    if(0 == response.result().errcode())
    {
        std::cout<<"rpc login response : success:"<<response.success()<<std::endl;
    }
    else{
        std::cout<<"rpc login response : errmsg:"<<response.result().errmsg()<<std::endl;
    }

    //演示调用远程发布的rpc方法Register
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("9999999");
    fixbug::RegisterResponse resp;
    stub.Register(nullptr,&req,&resp,nullptr);
    //调用完成，取结果
    if(0 == resp.result().errcode())
    {
        std::cout<<"rpc Register response : success:"<<resp.success()<<std::endl;
    }
    else{
        std::cout<<"rpc Register response : errmsg:"<<resp.result().errmsg()<<std::endl;
    }
    return 0;

}