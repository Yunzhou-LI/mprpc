#include "zookeeperutil.h"
#include "mprpcapplication.h"

//全局的观察watcher,zkserver给zkclient的通知
void global_watcher(zhandle_t *zh,int type,int state,const char* path,void *watcherCtx)
{
    if(type == ZOO_SESSION_EVENT)   //回调的消息类型是否和会话相关的消息类型
    {
        if(state == ZOO_CONNECTED_STATE)
        {
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }

    }
}
ZkClient::ZkClient():m_zhandle(nullptr){}
ZkClient::~ZkClient()
{
    if(m_zhandle != nullptr){
            zookeeper_close(m_zhandle);//关闭句柄，释放资源
    }
}
//zkclient启动连接zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeper_ip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeper_port");
    std::string connstr = host+":"+port;

    /*
    zookeeper_mt:多线程版本
    zookeeper的API客户端程序提供了三个线程
    API调用线程
    网络I/O线程 pthread _create poll
    watcher回调线程
    */
   //这里zookeeper的连接是一个异步的过程，并不会阻塞等待连接。通过信号量阻塞等待回调线程更新连接状态
   m_zhandle = zookeeper_init(connstr.c_str(),global_watcher,30000,nullptr,nullptr,0);
   if(nullptr == m_zhandle)
   {
        std::cout<<"zookeeper_init error"<<std::endl;
        exit(EXIT_FAILURE);
   }

   sem_t sem;
   sem_init(&sem,0,0);
   zoo_set_context(m_zhandle,&sem);

   sem_wait(&sem);
   std::cout<<"zookeeper_init success"<<std::endl;
}

void ZkClient::Create(const char* path,const char* data,int datalen,int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    //判断path表示的znode结点是否存在，若存在，则不重复创建
    flag = zoo_exists(m_zhandle,path,0,nullptr);
    if(ZNONODE == flag)
    {
        //创建指定path的znode节点
        flag = zoo_create(m_zhandle,path,data,datalen,&ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen);
        if(flag == ZOK)
        {
            std::cout<<"znode create success... path:"<<path<<std::endl;
        }
        else{
            std::cout<<"flag:"<<flag<<std::endl;
            std::cout<<"znode create error... path:"<<path<<std::endl;
            exit(EXIT_FAILURE);
        }
        
    }
}
//在zkserver上根据指定的znode节点路径，获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char path_buffer[64];
    int bufferlen = sizeof(path_buffer);
    int flag = zoo_get(m_zhandle,path,0,path_buffer,&bufferlen,nullptr);
    if(ZOK == flag)
    {
        return path_buffer;
    }
    else
    {
        std::cout<<"get znode error... path:"<<path<<std::endl;
        return "";
    }
}