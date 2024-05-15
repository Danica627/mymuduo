#pragma once
#include "Timestamp.h"
#include "Poller.h"

#include <vector>
#include <sys/epoll.h>

/**
 * epoll的使用
 * epoll_create
 * epoll_ctl  add/mod/del
 * epoll_wait
*/
class EPollPoller:public Poller
{
public:
    EPollPoller(EventLoop* loop);//epoll_create
    ~EPollPoller()override;

    //重写基类Poller的抽象方法
    Timestamp poll(int timeoutMs,ChannelList* activeChannels)override;//epoll_wait
    void updateChannel(Channel* channel)override;//epoll_ctl
    void removeChannel(Channel* channel)override;//epoll_ctl
private:
    static const int kInitEventListSize=16;//给vector<epoll_event>初始化的长度
    using EventList=std::vector<epoll_event>;

    //填写活跃的连接
    void fillActiveChannels(int numEvents,ChannelList* activeChannels)const;

    //更新channel通道
    void update(int operation,Channel* channel);

    int epollfd_;
    EventList events_;
};