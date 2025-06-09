#include<iostream>
#include"mprpcconfig.h"

 //负责解析配置文件
void MpRpcConfig::LoadConfigFile(const char * config_file)
{
    FILE* fd = fopen(config_file,"r");
    if(nullptr == fd){
        std::cout<<config_file<<" is not exist!"<<std::endl;
        exit(EXIT_FAILURE);
    }
    // 按行读取 会有三种情况  1.遇到 “#”  2.遇到"  "  3. 正常数据
    while(!feof(fd))
    {
        char buf[512];
        fgets(buf,512,fd);
        std::string src_buf(buf);
        int start_idx = src_buf.find_first_not_of(' ');
        int end_idx = src_buf.find_last_not_of(' ');
        if(start_idx != -1 && end_idx != -1)
        {
            src_buf = src_buf.substr(start_idx,end_idx+1);
        }
        if(src_buf[0] == '#' || src_buf.empty() ||src_buf[0] == '\n'){continue;}
        int idx = src_buf.find('=');
        if(idx == -1)
        {
            continue;
        }
        std::string key = src_buf.substr(0,idx);
        std::string value = src_buf.substr(idx+1,src_buf.size() - idx-1);
        int n_idx = value.find('\n');
        value = value.substr(0,n_idx);
        int last_value_idx = value.find_last_not_of(' ');
        value = value.substr(0,last_value_idx+1);
        m_ConfigMap.insert({key,value});
    }

}
//查看配置项信息
std::string MpRpcConfig::Load(std::string key)
{
    auto iter = m_ConfigMap.find(key);
    if(iter != m_ConfigMap.end())
    {
        return iter->second;
    }
    return "";  
}