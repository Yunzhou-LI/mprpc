#pragma once

#include<string>
#include<unordered_map>
class MpRpcConfig
{
public:
    //负责解析配置文件
    void LoadConfigFile(const char * config_file);
    //查看配置项信息
    std::string Load(std::string key);
private:
    std::unordered_map<std::string,std::string> m_ConfigMap;
};