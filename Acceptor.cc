#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

static int createNonblocking()//创建非阻塞的I/O 
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create err:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
}

//监听新用户的连接，返回跟客户端通信的socket，打包成TcpConnection,注册相应的回调
Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    :loop_(loop)
    ,acceptSocket_(createNonblocking())//创建socket
    ,acceptChannel_(loop,acceptSocket_.fd())//channel和poller都是通过请求本线程的loop和poller通信 
    ,listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);//bind绑定套接字 
    //TcpServer::start() Acceptor.listen  如果有新用户的连接，就要执行一个回调（connfd=》打包成channel=》唤醒subloop）
    //baseLoop => acceptChannel_(listenfd)有事件发生 => 底层反应堆调用回调 
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));//绑定回调 
}
Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listenning_=true;
    acceptSocket_.listen();//listen
    acceptChannel_.enableReading();//acceptChannel_=>Poller
}

//listenfd有事件发生，就是有新用户连接了
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd=acceptSocket_.accept(&peerAddr);
    if(connfd>=0)
    {
        if(NewConnectionCallback_)
        {
            NewConnectionCallback_(connfd,peerAddr);//轮询找到subloop,唤醒，分发当前的新客户端的channel
        }
        else//客户端没有办法去服务 
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept err:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        if(errno==EMFILE)//EMFILE错误的解决方法：1、调整当前进程文件描述符的上限 2、说明单台服务器不足以支撑现有的流量，进行集群和分布式部署
        {
            LOG_ERROR("%s:%s:%d sockfd reached limit\n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}