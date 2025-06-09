#include "rpcprovider.h"
#include "mprpcapplication.h"
#include<string>
#include<functional>
#include "rpcheader.pb.h"
#include "zookeeperutil.h"
#include<google/protobuf/descriptor.h>
//这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor* pServiceDec = service->GetDescriptor();
    //获取服务的名字
    std::string service_name = pServiceDec->name();
    //获取服务对象的方法数量
    int methodCnt = pServiceDec->method_count();

    std::cout<<"service_name:"<<service_name<<std::endl;
    ServiceInfo service_info;
    for(int i = 0;i<methodCnt;++i)
    {
        //获取服务对象指定下标的服务方法描述（抽象描述）
        const google::protobuf::MethodDescriptor* pmethodDes = pServiceDec->method(i);
        std::string m_name = pmethodDes->name();
        service_info.m_methodMap[m_name] = pmethodDes;

        std::cout<<"m_name:"<<m_name<<std::endl;
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name,service_info});
}
//启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserver_ip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserver_port").c_str());
    muduo::net::InetAddress address(ip,port);

    //创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");
    //绑定连接回调和消息读写回调方法
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    //设置muduo网络库的线程数量  1个IO线程    3个任务线程   
    server.setThreadNum(4);

    //把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    //session timeout:30s     zkClient 网络IO线程   1/3 *timeout 发送ping心跳消息
    ZkClient zkCli;
    zkCli.Start();
    for(const auto &sp:m_serviceMap)
    {
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0,0);
        for(const auto & mp:sp.second.m_methodMap)
        {
            std::string method_name = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            zkCli.Create(method_name.c_str(),method_path_data,sizeof(method_path_data),ZOO_EPHEMERAL);
        }
    }
    std::cout<<"RPC service start at ip:"<<ip<<" port:"<<port<<std::endl;
    //启动网络服务
    server.start();
    m_eventLoop.loop();

}
void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {
        conn->shutdown();
    }
}
/*
在框架内部，RpcProvider和RpcConsumer协商之间通信用的protobuf数据类型
包括： service_name  method_name   args_size  定义proto的message类型，进行数据头的序列化和反序列化
*/
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr& conn,muduo::net::Buffer*buffer,muduo::Timestamp)
{
    //网络上接收远程rpc调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();
    //从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size,4,0);

    //根据header_size 读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4,header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else{
        //数据头反序列化失败
        std::cout<<"rpc_header_str:"<<rpc_header_str<<" parse failed!"<<std::endl;
        return;
    }
    //获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4+header_size,args_size);

    //打印调试信息
    std::cout<<"============================================="<<std::endl;
    std::cout<<"header_size:"<<header_size<<std::endl;
    std::cout<<"rpc_header_str:"<<rpc_header_str<<std::endl;
    std::cout<<"service_name:"<<service_name<<std::endl;
    std::cout<<"method_name:"<<method_name<<std::endl;
    std::cout<<"args_str:"<<args_str<<std::endl;

    //获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end())
    {
        std::cout<<service_name<<" is not exist!"<<std::endl;
        return;
    }
    auto m_it = it->second.m_methodMap.find(method_name);
    if(m_it == it->second.m_methodMap.end())
    {
        std::cout<<service_name<<":"<<method_name<<" is not exist!"<<std::endl;
        return;
    }
    //获取service对象
    google::protobuf::Service *service = it->second.m_service;
    //获取method对象
    const google::protobuf::MethodDescriptor* method = m_it->second;
    //生成rpc调用的请求request和响应response参数
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str))
    {
        std::cout<<"request parse error, content:"<<args_str<<std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();
    //给CallMethod提供回调方法
    google::protobuf::Closure* done = google::protobuf::NewCallback<RpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message*>(this,&RpcProvider::sendRpcResponse,conn,response);

    //在框架上根据远端RPC请求，调用当前rpc节点上发布的方法
    //相当于 new Userservice().Login(controller,request,response,done) 的抽象调用
    service->CallMethod(method,nullptr,request,response,done);
}
//Closure的回调方法，用于序列化rpc的响应和网络发送
void RpcProvider::sendRpcResponse(const muduo::net::TcpConnectionPtr& conn,google::protobuf::Message* message){
    std::string response_str;
    if(message->SerializeToString(&response_str))
    {
        //序列化成功后，将执行结果返回给调用方
        conn->send(response_str);
    }
    else{
        std::cout<<"serialize message error"<<std::endl;
    }
    conn->shutdown();
}