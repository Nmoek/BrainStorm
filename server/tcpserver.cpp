#include "tcpserver.h"
#include "macro.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <errno.h>

#include <string.h>
#include <sys/time.h>
#include <sys/epoll.h>

#define LISTEN_PORT     100

namespace BrainStorm
{
static auto g_logger = spdlog::get("BrainStorm");

/**
 * @brief 当前线程运行的服务器指针
 */
static thread_local TcpServer* s_this = nullptr;

TcpServer::TcpServer(uint64_t thread_num)
    :m_threadSum(thread_num)
    ,m_nextThread(0)
{
    // gettimeofday(&be_tv, nullptr);

    KIT_ASSERT(m_threadSum > 0);


    m_base = event_base_new();
    
    KIT_ASSERT(m_base);



    // m_epollFd = epoll_create(1);
    
    // KIT_ASSERT2(m_epollFd, "epoll create error");

    // memset(events, 0, sizeof(struct epoll_event) * 512);


}


// 监听函数
int TcpServer::listen(uint16_t port, const std::string& ip)
{
    // 忽略管道信号 防止中断通信
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        g_logger->error("singal set error! errno={}, is:{}", errno, strerror(errno));
        return -1;
    }

    for(int i = 0;i < LISTEN_PORT;++i)
    {
        //创建套接字 socket
        int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(sock_fd < 0)
        {
            g_logger->error("socket create error!errno={}, is:{}", errno, strerror(errno));
            return -1;
        }

        // 将 socket 设置为 非阻塞
        evutil_make_socket_nonblocking(sock_fd);

        // 将socket 设置为 ip地址可复用
        evutil_make_listen_socket_reuseable(sock_fd);


        struct sockaddr_in addr;
        bzero(&addr, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port + i);

        //使用用户指定 的ip
        if(ip.size())
        {
            addr.sin_addr.s_addr = inet_addr(ip.c_str());
        }
        else  //否则默认
        {
            addr.sin_addr.s_addr = 0;
        }


        //服务器 ip port 绑定
        if(bind(sock_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
        {
            g_logger->error("bind address error! errno={}, is:{}", errno, strerror(errno));
            return -1;
        }


        //将套接字 由主动化为被动
        if(::listen(sock_fd, 5) < 0)
        {
            g_logger->error("listen error! errno={}, is:{}", errno, strerror(errno));
            return -1;
        }

 
        m_listenEv = event_new(m_base, sock_fd, EV_READ | EV_PERSIST, &TcpServer::StartAccept, this);
        if(event_add(m_listenEv, nullptr) < 0)
        {
            g_logger->error("add event to libevent error! errno={}, is:{}", errno, strerror(errno));
            return -1;
        }

    }

    g_logger->info("listen finish");

        //逐一开启线程数组的线程工作 即:开启线程池工作
    for(int i = 0;i < m_threadSum;i++)
    {
        Thread::ptr t(new Thread(this));
        t->start();
        m_threads.push_back(t);
    }
    g_logger->info("threads start");

    return 0;
}

//开启服务器 事件循环
void TcpServer::start()
{
    setThis();
    int ret = event_base_dispatch(m_base);
    if(ret == 1)
    {
        g_logger->info("server normal exit....");
    }
    else if(!ret)
    {
        g_logger->info("server start");
    }
    else
    {
        g_logger->error("server start fail!");
    }

    event_free(m_listenEv);
    event_base_free(m_base);
    
}


// //事件回调函数 负责处理新连接
void TcpServer::StartAccept(evutil_socket_t sock_fd, short what, void *arg)
{

    // 接受来自客户端 ip port 信息的 socket_in
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));

    socklen_t ac_len = sizeof(struct sockaddr_in);
    evutil_socket_t ac_fd = accept(sock_fd, (struct sockaddr *)&addr, &ac_len);

    if(ac_fd < 0 && errno != EAGAIN)
    {
        g_logger->error("accept client error! errno={}, is:{}", errno, strerror(errno));
        return;
    }

    TcpServer* server = (TcpServer *)arg;

    server->handleClient(ac_fd, std::string(inet_ntoa(addr.sin_addr)), ntohs(addr.sin_port));

}


void TcpServer::handleClient(int sock_fd, const std::string& ip, uint16_t port)
{
    auto child = m_threads[m_nextThread];

    //取完内核表 轮询到下一个线程  简易的负载均衡 平均分配
    m_nextThread = (m_nextThread + 1) % m_threadSum;

    SockItem si;
    si.ac_fd = sock_fd;
    si.ip = ip;
    si.port = port;

    int ret = write(child->getPipeWrite(), &si, sizeof(struct SockItem));
    if(ret < 0)
    {
        g_logger->error("write pipe error! errno={}, is:{}", errno, strerror(errno));
        return;
    }


}


void TcpServer::setThis()
{
    s_this = this;
}

TcpServer* TcpServer::GetThis()
{
    return s_this;
}


}





