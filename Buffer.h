#pragma once

#include <vector>
#include <string>
#include <algorithm>

//网络库底层的缓冲区类型定义
class Buffer
{
public:
    static const size_t kCheapPrepend=8;//记录数据包的长度
    static const size_t kInitialSize=1024;//缓冲区的大小

    explicit Buffer(size_t initialSize=kInitialSize)
        :buffer_(kCheapPrepend+initialSize)
        ,readerIndex_(kCheapPrepend)
        ,writerIndex_(kCheapPrepend)
    {}

    size_t readableBytes() const//可读的缓冲区大小
    {
        return writerIndex_-readerIndex_;
    }

    size_t writableBytes() const//可写的缓冲区大小
    {
        return buffer_.size()-writerIndex_;
    }

    size_t prependableBytes() const//prependable Bytes区域大小
    {
        return readerIndex_;
    }

    //返回缓冲区中可读数据的起始地址
    const char* peek() const
    {
        return begin()+readerIndex_;
    }

    //注册回调  onMessage 有读写事件发生时，将buffer->string
    void retrieve(size_t len)
    {
        if(len<readableBytes())
        {
            readerIndex_+=len;//应用只读取了可读缓冲区数据的一部分，就是len，还剩下readerIndex_+=len~writerIndex_没读
        }
        else//len==readableBytes()
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_=writerIndex_=kCheapPrepend;
    }

    //把onMessage函数上报的Buffer数据，转成string类型的数据返回
    std::string retrieveAllAsString()
    {
        return retrieveAllAsString(readableBytes());//应用可读取数据的长度
    }

    std::string retrieveAllAsString(size_t len)
    {
        std::string result(peek(),len);//peek() 可读数据的起始地址
        retrieve(len);//上面一句把缓冲区中可读的数据，已经读取出来，这里肯定要对缓冲区进行复位操作
        return result;
    }

    //剩余的可写缓冲区buffer_.size()~writerIndex_    要写的数据的长度  len
    void ensureWriteableBytes(size_t len)
    {
        if(writableBytes()<len)
        {
            makeSpace(len);//扩容函数
        }
    }

    //把[data,data+len]内存上的数据，添加到writable缓冲区当中
    void append(const char* data,size_t len)
    {
        ensureWriteableBytes(len);//确保空间可用
        std::copy(data,data+len,beginWrite());
        writerIndex_+=len;
    }

    char* beginWrite()//可以写的地方的地址
    {
        return begin()+writerIndex_;
    }

    const char* beginWrite()const//常对象可调用
    {
        return begin()+writerIndex_;
    }

    //从fd上读取数据
    ssize_t readFd(int fd,int* saveErrno);

    //通过fd发送数据
    ssize_t writeFd(int fd,int* saveErrno);

private:
    char* begin()
    {
        //it.operator*()  取迭代器访问的第一个元素，再对这个元素取地址
        return &*buffer_.begin();//vector底层数组首元素的地址，也就是数组的起始地址
    }
    const char* begin() const
    {
        return &*buffer_.begin();
    }

    void makeSpace(size_t len)
    {
        /*
        kCheapPrepend  |  reader    |     writer     |
        kCheapPrepend  |           len               |
        */
        if(writableBytes()+prependableBytes()<len+kCheapPrepend)//writableBytes()+prependableBytes()=>可写缓存区长度+已读完数据已经空闲下来的缓冲区
        {
            buffer_.resize(writerIndex_+len);//扩容
        }
        else//将未读数据和可写缓冲区往前挪
        {
            size_t readalbe=readableBytes();//未读数据长度
            std::copy(begin()+readerIndex_,//将未读数据挪到前面
                    begin()+writerIndex_,
                    begin()+kCheapPrepend);
            readerIndex_=kCheapPrepend;//readerIndex_前移
            writerIndex_=readerIndex_+readalbe;//writerIndex_前移
        }
    }
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};