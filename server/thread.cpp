#include "thread.h"
#include "macro.h"
#include "util.h"
#include "tcpserver.h"
#include "myserver.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <errno.h>
#include <iostream>

namespace BrainStorm
{

static std::shared_ptr<spdlog::logger> g_logger = spdlog::get("BrainStorm");


static thread_local Thread* s_thread_this = nullptr;
static thread_local TcpServer* s_server = nullptr;


Thread::Thread(TcpServer *server)
    :m_server(server)
{
    // 创建slibevent句柄
    m_base = event_base_new();
    if(!m_base)
    {
        g_logger->error("event_base_new error\n");
        KIT_ASSERT2(false, "event base create error");
    }

    // 创建管道
    if(pipe(m_pipe) < 0)
    {
        g_logger->error("create pipe error! errno={}, is:{}\n", errno, strerror(errno));
        KIT_ASSERT2(false, "pipe create error");
    }

}

Thread::~Thread()
{

    if(m_pipeBev)
        bufferevent_free(m_pipeBev);
    
    close(m_pipe[1]);
    
}


// 线程开启函数
void Thread::start()
{
    
    m_pipeBev = bufferevent_socket_new(m_base, m_pipe[0], BEV_OPT_CLOSE_ON_FREE);
    if(!m_pipeBev)
    {
        g_logger->error("bufferevent create error");
        event_base_free(m_base);
        return;
    }

    bufferevent_setcb(m_pipeBev, &Thread::PipeReadCB, nullptr, nullptr, this);

    //将缓冲区 使能化 可读 可写 可接受信号
    bufferevent_enable(m_pipeBev, EV_READ);

    //创建线程  C API  将当前 对象的指针this 传给 ThreadCallBack
    if(pthread_create(&m_threadId, nullptr, &Thread::ThreadCallBack, this) < 0)
    {
        g_logger->error("create thread fail!, error={}, is={}\n", errno, strerror(errno));
        KIT_ASSERT2(false, "thread create error!");
    }
    
    // 分离态   不由主线程回收资源 由系统负责回收
    pthread_detach(m_threadId);
}

void* Thread::ThreadCallBack(void *arg)
{
    // 将 操作重新回到 普通成员函数中 为了使用非静态成员变量
    Thread *p = (Thread *)arg;
    s_thread_this = p;
    s_server = p->m_server;
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
void Thread::PipeReadCB(struct bufferevent *m_bev, void* ctx)
{

    Thread *t = (Thread*)ctx;

    SockItem* si = new SockItem;

    bufferevent_read(t->m_pipeBev, si, sizeof(struct SockItem));

    struct bufferevent* bev = bufferevent_socket_new(t->m_base, si->ac_fd, BEV_OPT_CLOSE_ON_FREE);
    if(!bev)
    {
        g_logger->error("bufferevent create error");
        event_base_loopbreak(t->m_base);
        return;
    }

    /* 封装一个通信类 来完成数据收发*/
    TcpSocket::ptr s(new TcpSocket(bev, si->ip, si->port, si->ac_fd));

    //给对应线程添加客户端连接
    t->addClint(si->ac_fd, s);

    // 设置 缓冲区回调函数 从 通信类 TcpSocket 中调用
    bufferevent_setcb(bev, s->BufferReadCB, nullptr, s->BufferEventCB, s.get());

    //将缓冲区 使能化 可读 可写 可接受信号
    bufferevent_enable(bev, EV_READ | EV_WRITE);


}

void Thread::addClint(int fd, TcpSocket::ptr s)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_clients.insert({fd, s});
}


bool Thread::delClient(int fd)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_clients.find(fd);
    if(it == m_clients.end())
        return false;
    
    m_clients.erase(it);
    return true;
}



Thread* Thread::GetThis()
{
    return s_thread_this;
}


TcpServer* Thread::GetServer()
{
    return dynamic_cast<MyServer*>(s_server);
}

}
