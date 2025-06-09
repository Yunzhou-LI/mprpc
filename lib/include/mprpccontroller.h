#pragma once 
#include <google/protobuf/service.h>
#include <string>

class MpRpcController:public google::protobuf::RpcController
{
public:
    MpRpcController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);
    //以下函数仅重写，实现为空
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);
private:
    bool m_failed;  //RPC方法执行过程中的状态
    std::string m_errText;  //RPC方法执行过程中的错误信息
};