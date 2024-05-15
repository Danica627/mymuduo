#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>
std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func,const std::string& name)//构造函数
    :started_(false)
    ,joined_(false)
    ,tid_(0)
    ,func_(std::move(func))//将func底层的资源直接转给成员变量func_，效率更高
    ,name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started_&&!joined_)//线程运行起来了，就需要析构，如果线程没运行起来，就什么也不用做
    {
        //thread类提供的设置分离线程的方法
        thread_->detach();//成了一个守护线程，当主线程结束，守护线程会自动结束，不用担心孤儿线程的出现会泄露内核资源
        //detach和join不能同时作用，join相当于调用它的这个线程是一个工作线程，主线程必须要等待工作线程结束，再继续执行
    }
}

void Thread::start()//启动当前线程 ,一个Thread对象，记录的就是一个新线程的详细信息
{
    started_=true;
    sem_t sem;
    sem_init(&sem,false,0);

    //开启线程
    thread_=std::shared_ptr<std::thread>(new std::thread([&](){
        //获取线程的tid值
        tid_=CurrentThread::tid();
        sem_post(&sem);
        //开启一个新线程，专门执行该线程函数
        func_();
    }));

    //这里必须等待获取上面新创建的线程的tid
    sem_wait(&sem);
}
void Thread::join()//当前线程等待其他线程完了再运行下去 
{
    joined_=true;
    thread_->join();
}


void Thread::setDefaultName()//给线程设置默认的名称
{
    int num=++numCreated_;
    if(name_.empty())
    {
        char buf[32]={0};
        snprintf(buf,sizeof buf,"Thread%d",num);
        name_=buf;
    }
}