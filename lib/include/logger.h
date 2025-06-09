#pragma once 
#include "lockqueue.h"
#include<string>

enum LogLevel
{
    INFO,
    ERROR
};
//Mprpc框架提供的日志系统
class Logger
{
public:
    //获取唯一单例
    static Logger& GetInstance();
    //设置日志级别
    void setLogLevel(LogLevel lvl);
    //写日志
    void Log(std::string msg);
private:
    Logger();
    Logger(const Logger &) =delete;
    Logger(Logger &&) = delete;
    int m_lvl;
    LockQueue<std::string> m_lckQue; //日志缓冲队列
};

//定义宏
#define LOG_INFO(logmsgformat,...)  \
    do{ \
        Logger &logger = Logger::GetInstance(); \
        logger.setLogLevel(LogLevel::INFO);     \
        char c[1024] = {0};                     \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__);      \
        logger.Log(c);                                    \
    } while (0);    
#define LOG_ERR(logmsgformat,...)  \
    do{ \
        Logger &logger = Logger::GetInstance(); \
        logger.setLogLevel(LogLevel::ERROR);     \
        char c[1024] = {0};                     \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__);      \
        logger.Log(c);                                    \
    } while (0);  
    
    