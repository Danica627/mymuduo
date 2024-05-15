#pragma once

#include "noncopyable.h"
#include "Thread.h"

#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>

class EventLoop;

class EventLoopThread:noncopyable
{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb=ThreadInitCallback(), //线程初始化的回调 
        const std::string& name=std::string());
    ~EventLoopThread();

    EventLoop* startLoop();//开启循环 
private:
    void threadFunc();//线程函数，创建loop 

    EventLoop* loop_;
    bool exiting_;//是否退出循环
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;//初始化操作 
};