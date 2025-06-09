#include<iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"


int main(int argc,char** argv)
{
    //整个程序启动以后，想使用rpc框架来享受rpc服务，一定要先调用框架初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);

    //演示调用远程发布的rpc方法Login
    fixbug::FriendServcieRpc_Stub stub(new MprpcChannel());
    //rpc方法的请求参数
    fixbug::GetFriendListRequest request;
    request.set_userid(22);
    //rpc方法的响应参数
    fixbug::GetFriendListResponse response;
    MpRpcController controller;
    stub.GetFriendList(&controller,&request,&response,nullptr);   //RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送
    //调用完成，取结果
    if(controller.Failed())
    {
        std::cout<<controller.ErrorText()<<std::endl;
    }
    else{
        if(0 == response.result().errcode())
        {
            int num = response.friends_size();
            for(int i = 0;i<num;i++)
            {
                std::cout<<"name:"<<response.friends(i)<<std::endl;
            }
        }
        else{
            std::cout<<"rpc GetFriendList response : errmsg:"<<response.result().errmsg()<<std::endl;
        }
        return 0;

    }
    
}