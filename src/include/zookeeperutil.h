#pragma once 
#include<string>
#include<semaphore.h>
#include<zookeeper/zookeeper.h>

//封装zk的客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    //zkclient启动连接zkserver
    void Start();
    //参数state用于区分是临时性节点还是永久性节点 。0：永久
    void Create(const char* path,const char* data,int datalen,int state);
    //在zkserver上根据指定的znode节点路径，获取znode节点的值
    std::string GetData(const char *path);
private:
    //zk的客户端句柄
    zhandle_t* m_zhandle;
};