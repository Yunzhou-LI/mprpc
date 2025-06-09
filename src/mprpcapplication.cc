#include "mprpcapplication.h"
#include<iostream>
#include<string>
#include "unistd.h"

//静态成员初始化
MpRpcConfig MprpcApplication::m_config;
void showArgsHelp()
{
    std::cout<<"format: command -i <configfile>"<<std::endl;
}
MprpcApplication& MprpcApplication::GetInstance(){
    static MprpcApplication app;
    return app;
}
void MprpcApplication::Init(int argc,char** argv)
{
    if(argc < 2){
        showArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    //
    while((c = getopt(argc,argv,"i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            /* code */
            config_file = optarg;
            break;
        case '?':
            std::cout<<"invalid args"<<std::endl;
            showArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            std::cout<<"need <configfile>"<<std::endl;
            showArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    //开始加载配置文件 
    //配置文件： rpcserver_ip=xxx  rpcserver_port=xxx zookeeper_ip=xxx  zookeeper_port=xxx
    m_config.LoadConfigFile(config_file.c_str());
    std::string rpc_ip("rpcserver_ip");
    std::string rpc_port("rpcserver_port");
    std::string zk_ip("zookeeper_ip");
    std::string zk_port("zookeeper_port");
}
MpRpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}