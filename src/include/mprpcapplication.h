#pragma once
#include "mprpcchannel.h"
#include "mprpccontroller.h"
#include "mprpcconfig.h"
//mprpc框架的基础类，负责框架的一些初始化操作
class MprpcApplication{
public:
    static MprpcApplication& GetInstance();
    static void Init(int argc,char** argv);
    static MpRpcConfig& GetConfig();
private:
    static MpRpcConfig m_config;
    MprpcApplication(){};
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication( MprpcApplication &&) = delete;
};