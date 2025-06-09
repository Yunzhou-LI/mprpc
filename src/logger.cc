#include "logger.h"
#include <thread>
#include<time.h>
#include<iostream>
//获取唯一单例
Logger & Logger::GetInstance()
{
    static Logger log;
    return log;
}
Logger::Logger(){
    std::thread writeLogTask([&](){
        for(;;)
        {
            //获取当前的日期，然后取日志信息，写入相应的文件中
            time_t now = time(nullptr);
            tm * nowtm=localtime(&now);

            char file_name[128];
            snprintf(file_name,sizeof(file_name),"%d-%d-%d-log.txt",nowtm->tm_year + 1900,nowtm->tm_mon + 1,nowtm->tm_mday);
            FILE*fp = fopen(file_name,"a+");    //appending模式
            if(fp == nullptr)
            {
                std::cout<<"logger file:"<<file_name<<" open error!"<<std::endl;
                exit(EXIT_FAILURE);
            }
            std::string msg = m_lckQue.Pop();

            char time_buf[128] = {0};
            sprintf(time_buf,"%d:%d:%d [%s]=> ",nowtm->tm_hour,nowtm->tm_min,nowtm->tm_sec,(m_lvl == INFO? "info":"error"));
            msg.insert(0,time_buf);
            msg.append("\n");
            fputs(msg.c_str(),fp);
            fclose(fp);
        }
    });
    //设置分离线程，守护线程
    writeLogTask.detach();
}
//设置日志级别
void Logger::setLogLevel(LogLevel lvl)
{
    m_lvl = lvl;
}
//写日志
void Logger::Log(std::string msg)
{
    m_lckQue.Push(msg);
}   