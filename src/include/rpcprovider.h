#pragma once
#include<memory>
#include<unordered_map>
#include<muduo/net/EventLoop.h>
#include<muduo/net/TcpServer.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include "google/protobuf/service.h"
//框架提供的专门发布rpc服务的网络对象类
class RpcProvider
{
public:
    //这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);
    //启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();
private:
    //组合EventLoop
    muduo::net::EventLoop m_eventLoop;
    //service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap; //保存服务名及其方法
    };
    //存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string,ServiceInfo> m_serviceMap; 
    //新的socket连接回调
    void onConnection(const muduo::net::TcpConnectionPtr&);
    //消息回调
    void onMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp);
    //Closure的回调方法，用于序列化rpc的响应和网络发送
    void sendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message* );
};