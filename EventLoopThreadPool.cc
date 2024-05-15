#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

#include <memory>

 EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,const std::string& nameArg)
    :baseLoop_(baseLoop)
    ,name_(nameArg)
    ,started_(false)
    ,numThreads_(0)
    ,next_(0)
 {}
EventLoopThreadPool::~EventLoopThreadPool()
{}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    started_=true;

    for(int i=0;i<numThreads_;i++)
    {
        char buf[name_.size()+32];
        snprintf(buf,sizeof buf,"%s%d",name_.c_str(),i);
        EventLoopThread* t=new EventLoopThread(cb,buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));//不想手动delete
        loops_.push_back(t->startLoop());//底层创建线程，绑定一个新的EventLoop，并返回该loop的地址
    }

    //整个服务端只有一个线程，运行着baseloop,就是用户创建的mainloop
    if(numThreads_==0&&cb)
    {
        cb(baseLoop_);
    }
}

//如果工作在多线程中，baseloop_默认以轮询的方式分配channel给subloop
//通过轮询的方式从子线程中取loop（循环）
//IO线程  baseloop  用作处理用户的连接事件
//工作线程  新创建的loop  用于处理用户的读写事件
EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop* loop=baseLoop_;//用户创建的mainloop

    if(!loops_.empty())//通过轮询获取下一个处理事件的loop
    {
        loop=loops_[next_];
        next_++;
        if(next_>=loops_.size())
        {
            next_=0;
        }
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    if(loops_.empty())
    {
        return std::vector<EventLoop*>(1,baseLoop_);
    }
    else
    {
        loops_;
    }
}