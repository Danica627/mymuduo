#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    extern __thread int t_cachedTid;

    void cacheTid();

    inline int tid()//内联函数，只在当前文件起作用
    {
        if(__builtin_expect(t_cachedTid==0,0))//缓存中没有这个tid
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}