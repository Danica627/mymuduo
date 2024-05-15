#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>
class EventLoop;//定义类型前置声明，因为只用EventLoop定义指针，指针大小固定，所以不用包含全部的头文件

/**
 * 理清楚 EventLoop、Channel、Poller之间的关系  《=Reactor模型上对应Demultiplex
 * Channel 理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN、EPOLLOUT事件
 * 还绑定了poller返回的具体事件
*/

class Channel
{
public:
    using EventCallback=std::function<void()>;
    using ReadEventCallback=std::function<void(Timestamp)>;

    Channel(EventLoop* loop,int fd);
    ~Channel();

    //fd得到poller通知以后，处理事件的。（如何处理事件：调用相应的回调方法）
    void handleEvent(Timestamp receiveTime);

    //设置回调函数对象
    void setReadCallback(ReadEventCallback cb){readCallback_=std::move(cb);}//将cb转为右值，即直接将cb的资源转给readCallback_，因为cb出了这句就没有用了
    void setWriteCallback(EventCallback cb){writeCallback_=std::move(cb);}
    void setCloseCallback(EventCallback cb){closeCallback_=std::move(cb);}
    void setErrorCallback(EventCallback cb){errorCallback_=std::move(cb);}

    //防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd()const{return fd_;}//返回了fd这个成员变量的值
    int events()const{return events_;}//返回了fd所感兴趣的事件
    int set_revents(int revt){revents_=revt;}//poller监听事件，设置了channel的fd相应事件 
    

     //设置fd相应的事件状态，要让fd对这个事件感兴趣 
     //update()通知poller调用epoll_ctl将fd感兴趣的事件添加到epoll里面
    void enableReading(){events_|=kReadEvent;update();}//用的是位运算，读事件对应的位是1，或上去之后0|1=>1,将读事件添加上
    void disableReading(){events_&=~kReadEvent;update();}//取反再与，去掉读事件，取反后，读事件对应位0，其它位1，1&0=>0,1&1=>1,读位变0，其它位不受影响
    void enableWriting(){events_|=kWriteEvent;update();}
    void disableWriting(){events_&=~kWriteEvent;update();}
    void disableAll(){events_=kNoneEvent;update();}

    //返回fd当前的事件状态
    bool isNoneEvent()const{return events_==kNoneEvent;}//当前channel的fd有没有注册感兴趣的事件
    bool isWriting()const{return events_&kWriteEvent;}
    bool isReading()const{return events_&kReadEvent;}

    //for Poller
    int index(){return index_;}
    void set_index(int idx){index_=idx;}

    //one loop per thread
    EventLoop* ownerLoop(){return loop_;}
    void remove();//删除Channel
private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);//受保护的事件处理

    //对当前fd感兴趣的状态的描述
    static const int kNoneEvent;//没有感兴趣的事件
    static const int kReadEvent;//对读事件感兴趣
    static const int kWriteEvent;//对写事件感兴趣

    EventLoop* loop_;//事件循环,Channel属于哪个EventLoop
    const int fd_;//fd,Poller监听的对象
    int events_;//注册fd感兴趣的事件
    int revents_;//poller返回的具体发生的事件
    int index_;//表示channel在poller中的状态 -1、1、2

    std::weak_ptr<void> tie_;//绑定自己,观察channel是否被remove
    bool tied_;

    //因为channel通道里面能够获知fd最终发生的具体的事件revents,所以它负责调用具体事件的回调操作
    //这些回调是用户设定的，通过接口传给channel来负责调用 ，channel才知道fd上是什么事件 
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};