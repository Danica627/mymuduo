#include "EventLoopThread.h"
#include "EventLoop.h"


EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, //线程初始化的回调 
        const std::string& name)
        :loop_(nullptr)
        ,exiting_(false)
        ,thread_(std::bind(&EventLoopThread::threadFunc,this),name)//绑定回调函数
        ,mutex_()
        ,cond_()
        ,callback_(cb)
        {

        }
EventLoopThread::~EventLoopThread()
{
    exiting_=true;
    if(loop_!=nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()//开启循环 
{
    thread_.start();//启动底层的新线程
	//启动后执行的是EventLoopThread::threadFunc

    EventLoop* loop=nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_==nullptr)//loop指针还没有初始化
        {
            cond_.wait(lock);////挂起，等待
        }
        loop=loop_;
    }
    return loop;
}

//下面这个方法，是在单独的新线程里面运行的
void EventLoopThread::threadFunc()//线程函数，创建loop 
{
    EventLoop loop;//创建一个独立的eventloop，和上面的线程是一一对应的，one loop per thread

    if(callback_)//如果有回调
    {
        callback_(&loop);//绑定loop做一些事情
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=&loop;//就是运行在这个线程的loop对象,将这个对象初始化好之后(loop指针指向loop对象)，才能唤醒（通知）
        cond_.notify_one();//唤醒1个线程，被唤醒后去访问loop指针
    }
    loop.loop();//EventLoop loop=>Poller.poll
    std::unique_lock<std::mutex> lock(mutex_);
    loop_=nullptr;
}
