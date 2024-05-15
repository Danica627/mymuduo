#pragma once
#include "Timestamp.h"

#include <memory>
#include <functional>

class Buffer;
class TcpConnection;

using TcpConnectionPtr=std::shared_ptr<TcpConnection>;
using ConnectionCallback=std::function<void(const TcpConnectionPtr&)>;
using CloseCallback=std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback=std::function<void(const TcpConnectionPtr&)>;
using MessageCallback=std::function<void(const TcpConnectionPtr&,
                                        Buffer*,
                                        Timestamp)>;
using HighWaterMarkCallback=std::function<void(const TcpConnectionPtr&,size_t)>;//高水位回调
            //接收方与发送方两边的速度应该趋于一致，进行一个水位控制，到达水位线就暂停发送