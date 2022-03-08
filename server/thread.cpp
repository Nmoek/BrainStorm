#include "thread.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <errno.h>
#include "util.h"

namespace BrainStorm
{

static std::shared_ptr<spdlog::logger> g_logger = spdlog::get("BrainStorm");


Thread::Thread()
{
    // 创建slibevent句柄
    m_base = event_base_new();
    if(!m_base)
    {
        g_logger->error("event_base_new error\n");
        exit(-1);
    }

    // 创建管道
    if(pipe(m_pipe) < 0)
    {
        g_logger->error("create pipe error! errno={}, is:{}\n", errno, strerror(errno));
        exit(-1);
    }

    //为管道读端绑定回调函数 添加到epoll中
    m_pipeEvent = event_new(m_base, m_pipe[0], EV_READ | EV_PERSIST, PipeReadCB, this);

    event_add(m_pipeEvent, NULL);

}


// 线程开启函数
void Thread::start()
{

    //创建线程  C API  将当前 对象的指针this 传给 ThreadCallBack
    if(pthread_create(&m_threadId, nullptr, &Thread::ThreadCallBack, this) < 0)
    {
        g_logger->error("create thread fail!, error={}, is={}\n", errno, strerror(errno));
        exit(-1);
    }
    
    // 分离态   不由主线程回收资源 由系统负责回收
    pthread_detach(m_threadId);
}


// 表面 线程工作函数  何谓表面？在这里仅仅为了配合 C API 而不发挥真正的作用
void* Thread::ThreadCallBack(void *arg)
{
    // 将 操作重新回到 普通成员函数中 为了使用非静态成员变量
    Thread *p = (Thread *)arg;
    p->run();

    return nullptr;
}


// 真正的 线程工作函数
void Thread::run()
{
    // 监听 内核事件表
    // 开启 事件循环
    int ret = event_base_dispatch(m_base);
    pid_t id = GetThreadId();
    if(ret == 0)
    {
        g_logger->debug("thread={} normal exit.", id);
    }
    else if(!ret)
    {
        g_logger->debug("thread={} start", id);
    }
    else
    {
        g_logger->error("thread={} exception", id);
    }

    //结束后 释放内核事件表
    event_base_free(m_base);
}


//事件回调函数 负责处理管道有数据可读
void Thread::PipeReadCB(evutil_socket_t fd, short what, void *arg)
{
    g_logger->debug("thread id={}", (pid_t)pthread_self());
}



}
