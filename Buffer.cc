#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

/**
 * 从fd上读取数据  Poller工作在LT模式
 * Buffer缓冲区是有大小的！  但是从fd上读数据的时候  却不知道tcp数据最终的大小
*/
ssize_t Buffer::readFd(int fd,int* saveErrno)
{
    char extrabuf[65536]={0};//栈上的内存空间  64k

    struct iovec vec[2];

    const size_t writable=writableBytes();//这是Buffer底层缓冲区剩余的可写空间大小
    vec[0].iov_base=begin()+writerIndex_;//缓冲区的起始地址
    vec[0].iov_len=writable;//缓冲区的长度

    //如果vec[0]缓冲区够填的话，就填充到vec[0]，不够填，就填到vec[1],最后，如果我们看到vec[1]中有内容的话，就把其中的内容直接添加到缓冲区中
    //缓冲区刚刚好存入所有我们需要写入的内容，不浪费空间，空间利用率高
    vec[1].iov_base=extrabuf;
    vec[1].iov_len=sizeof extrabuf;

    const int iovcnt=(writable<sizeof extrabuf)?2:1;
    const ssize_t n=::readv(fd,vec,iovcnt);
    if(n<0)
    {
        *saveErrno=errno;
    }
    else if(n<=writable)//Buffer的可写缓冲区已经够存储读出来的数据了
    {
        writerIndex_+=n;
    }
    else//extrabuf里面也写入了数据
    {
        writerIndex_=buffer_.size();
        append(extrabuf,n-writable);//writerIndex_开始写n-writable大小的数据
    }
    
    return n;//读取的字节数
}

// 通过fd发送数据
ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n=::write(fd,peek(),readableBytes());
    if(n<0)//错误
    {
        *saveErrno=errno;
    }
    return n;
}
