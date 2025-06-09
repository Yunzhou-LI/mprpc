#include"friend.pb.h"
#include<vector>
#include<iostream>
#include<string>
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"

class FriendService:public fixbug::FriendServcieRpc
{
public:
    std::vector<std::string> GetFriendList(int userid)
    {
        std::cout<<"do GetFriendList service"<<std::endl;
        std::vector<std::string> vec;
        vec.push_back("gao yang");
        vec.push_back("yang shuo");
        vec.push_back("shan dong");
        return vec;
    }

    void GetFriendList(google::protobuf::RpcController* controller,
                         const ::fixbug::GetFriendListRequest*request,
                         ::fixbug::GetFriendListResponse*response,
                         ::google::protobuf::Closure* done)
    {
        uint32_t id = request->userid();
        std::vector<std::string> friendList = GetFriendList(id);
        for(const auto &name:friendList)
        {
            std::string *p = response->add_friends();
            *p = name;
        }
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        done->Run();
    }
};

int main(int argc,char **argv)
{
    //调用框架的初始化操作
    MprpcApplication::Init(argc,argv);

    LOG_ERR("%s:%s:%d",__FILE__,__FUNCTION__,__LINE__);
    LOG_INFO("first log message!");
    //
    RpcProvider provider;
    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    provider.NotifyService(new FriendService());
    //启动一个rpc网络服务发布节点，进入阻塞状态，等待远程的rpc调用请求
    provider.Run();
    return 0;
}