#pragma once

#include <string>

#include "noncopyable.h"

//LOG_INFO("%s %d",arg1,arg2)
//##__VA_ARGS__  获取可变参列表的宏
//logmsgFormat：字符串，后面...是可变参的意思
#define LOG_INFO(logmsgFormat, ...)\
do \
{ \
    Logger& logger=Logger::instance(); \
    logger.setLogLevel(INFO); \
    char buf[1024]={0}; \
    snprintf(buf,1024,logmsgFormat,##__VA_ARGS__); \
    logger.log(buf); \
}while(0)

#define LOG_ERROR(logmsgFormat, ...)\
do \
{ \
    Logger& logger=Logger::instance(); \
    logger.setLogLevel(ERROR); \
    char buf[1024]={0}; \
    snprintf(buf,1024,logmsgFormat,##__VA_ARGS__); \
    logger.log(buf); \
}while(0)

#define LOG_FATAL(logmsgFormat, ...)\
do \
{ \
    Logger& logger=Logger::instance(); \
    logger.setLogLevel(FATAL); \
    char buf[1024]={0}; \
    snprintf(buf,1024,logmsgFormat,##__VA_ARGS__); \
    logger.log(buf); \
    exit(-1);\
}while(0)

#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...) \
do \
{ \
    Logger& logger=Logger::instance(); \
    logger.setLogLevel(DEBUG); \
    char buf[1024]={0}; \
    snprintf(buf,1024,logmsgFormat,##__VA_ARGS__); \
    logger.log(buf); \
}while(0)
#else
    #define LOG_DEBUG(logmsgFormat, ...)
#endif

/**定义日志的级别  
 * INFO 打印正常的流程信息
 * ERROR 错误，但不影响软件继续向下执行
 * FATAL 毁灭性打击，出现后系统无法向下运行，就要输出关键的日志信息，然后exit
 * DEBUG 调试信息，默认关闭，可以加上一个宏来控制，如果运行程序时加上这个宏，就开启muduo库的调试信息
 */
enum LogLevel
{
    INFO, //普通信息
    ERROR, //错误信息
    FATAL, //core信息
    DEBUG, //调试信息
};

//输出一个日志类
class Logger:noncopyable
{
public:
    //获取日志唯一的实例对象
    static Logger& instance();
    //设置日志级别
    void setLogLevel(int level);
    //写日志
    void log(std::string msg);
private:
    int logLevel_;//后面加_  ：1、防止和库中的名字冲突，库中变量名大多在前面加_；2、区分函数内的变量（局部变量）
    Logger(){}
};