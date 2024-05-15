#pragma once

#include <iostream>
#include <string>

//时间类
class Timestamp
{
public:
    Timestamp();//默认构造
    explicit Timestamp(int64_t microSecondsSinceEpoch);//带参数的构造，带参数的构造函数都加了explicit关键字,避免隐式对象转换
    static Timestamp now();//获取当前时间
    std::string toString() const;//获取当前时间的年月日时分秒的输出

private:
    int64_t microSecondsSinceEpoch_;
};