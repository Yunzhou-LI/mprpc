#include "mprpcchannel.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include <string>
#include <sys/types.h>        
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "zookeeperutil.h"

/*
header_size + service_name + method_name + args_size + args
*/
//所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor * sd= method->service();
    std::string service_name = sd->name();  //service_name
    std::string method_name = method->name();   //method_name

    //获取参数的序列化字符串长度 args_size
    uint32_t args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else{
        controller->SetFailed("serialize request error");
        return;
    }

    //定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size = 0;
    std::string rpc_header_str;
    if(rpcHeader.SerializeToString(&rpc_header_str)){
        header_size = rpc_header_str.size();
    }
    else{
        controller->SetFailed("serialize header error");
        return;
    }

    //组织待发送的rpc请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0,std::string((char*)&header_size,4)); //header_size
    send_rpc_str+=rpc_header_str;
    send_rpc_str+=args_str;

     //打印调试信息
    std::cout<<"============================================="<<std::endl;
    std::cout<<"header_size:"<<header_size<<std::endl;
    std::cout<<"rpc_header_str:"<<rpc_header_str<<std::endl;
    std::cout<<"service_name:"<<service_name<<std::endl;
    std::cout<<"method_name:"<<method_name<<std::endl;
    std::cout<<"args_str:"<<args_str<<std::endl;

    //网络发送 使用tcp编程，完成rpc的远程调用
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    if(clientfd == -1)
    {
        char buffer[1024];
        snprintf(buffer,1024,"crate socket error! errno:%d",errno);
        controller->SetFailed(buffer);
        return;
    }
    //读取配置文件rpcserver信息
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserver_ip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserver_port").c_str());

    ZkClient zkCli;
    zkCli.Start();
    std::string method_path = "/" + service_name + "/" + method_name;
    std::string host_data = zkCli.GetData(method_path.c_str());

    if(host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if(idx == -1)
    {
        controller->SetFailed(method_path + "address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0,idx);
    uint16_t port = atoi(host_data.substr(idx+1,host_data.size() - idx - 1).c_str());
    std::cout<<ip<<"  "<<port<<std::endl;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    //连接rpc服务节点
    if(-1 == connect(clientfd,(struct  sockaddr*)&server_addr,sizeof(server_addr)))
    {
        close(clientfd);
        char buffer[1024];
        snprintf(buffer,1024,"connect error! errno:%d",errno);
        controller->SetFailed(buffer);
        return;
    }
    //发送rpc请求
    if(-1 == send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0))
    {
        close(clientfd);
        char buffer[1024];
        snprintf(buffer,1024,"send error! errno:%d",errno);
        controller->SetFailed(buffer);
        return;
    }
    //接收rpc请求的返回值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if(-1 == (recv_size = recv(clientfd,recv_buf,1024,0)))
    {
        close(clientfd);
        char buffer[1024];
        snprintf(buffer,1024,"recv error! errno:%d",errno);
        controller->SetFailed(buffer);
        return;
    }
    //对rpc响应进行反序列化
    if(!response->ParseFromArray(recv_buf,recv_size))
    {
        close(clientfd);
        char buffer[2048];
        snprintf(buffer,2048,"parse error! response_str:%s",recv_buf);
        controller->SetFailed(buffer);
        return;
    }
    close(clientfd);
    return;
}