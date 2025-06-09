#include<iostream>
#include<string>
#include "user.pb.h"
#include"mprpcapplication.h"
#include "rpcprovider.h"
/*
    UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendList
*/
class UserService :public fixbug::UserServiceRpc
{
    public:
    bool Login(std::string name,std::string pwd)
    {
        std::cout<<"doing local service: Login"<<std::endl;
        std::cout<<"name:"<<name<<" pwd:"<<pwd<<std::endl;
        return true;
    }
    bool Register(uint32_t id,std::string name,std::string pwd)
    {
        std::cout<<"doing local service: Register"<<std::endl;
        std::cout<<"id"<<id<<" name:"<<name<<" pwd:"<<pwd<<std::endl;
        return true;
    }

    void Login(google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done) //这里相当于服务端收到远程服务调用时的响应过程
    {
        //获取请求中的参数
        std::string name = request->name();
        std::string pwd = request->pwd();
        //做本地业务
        bool login_result = this->Login(name,pwd);
        // 将响应消息写入
        response->set_success(login_result);
        fixbug::ResultInfo *re_info = response->mutable_result();
        re_info->set_errcode(0);
        re_info->set_errmsg("");
        //执行回调操作，执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }
    void Register(google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done) //这里相当于服务端收到远程服务调用时的响应过程
    {
        //获取请求中的参数
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        //做本地业务
        bool register_result = this->Register(id,name,pwd);
        // 将响应消息写入
        response->set_success(register_result);
        fixbug::ResultInfo *re_info = response->mutable_result();
        re_info->set_errcode(0);
        re_info->set_errmsg("");
        //执行回调操作，执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }
};

int main(int argc,char **argv)
{
    //调用框架的初始化操作
    MprpcApplication::Init(argc,argv);

    //
    RpcProvider provider;
    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    provider.NotifyService(new UserService());
    //启动一个rpc网络服务发布节点，进入阻塞状态，等待远程的rpc调用请求
    provider.Run();
    return 0;
}