#include "tcpserver.h"
#include "macro.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <errno.h>

#include <string.h>
#include <sys/time.h>


namespace BrainStorm
{
static auto g_logger = spdlog::get("BrainStorm");

TcpServer::TcpServer(int thread_num)
    :m_threadSum(thread_num)
    ,next_thread_index(0)
{
    gettimeofday(&be_tv, nullptr);

    KIT_ASSERT(m_threadSum > 0);


    m_base = event_base_new();

    KIT_ASSERT(m_base);
}


// 监听函数
int TcpServer::listen(int port, const std::string& ip)
{
    // 忽略管道信号 防止中断通信
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        g_logger->error("singal set error! errno={}, is:{}", errno, strerror(errno));
        return -1;
    }

int t = 10;
while(t--)
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
    addr.sin_port = htons(port++);

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
    if(::listen(sock_fd, 10) < 0)
    {
        g_logger->error("listen error! errno={}, is:{}", errno, strerror(errno));
        return -1;
    }

    g_logger->info("listen....");

    //将被动套接字 与相应事件绑定 加入内核事件表监听
    std::shared_ptr<struct event> ev(event_new(m_base, sock_fd, EV_READ | EV_PERSIST, StartAccept, this));

    if(event_add(ev.get(), nullptr) < 0)
    {
        g_logger->error("add event to libevent error!");
        return -1;
    }
}


    //逐一开启线程数组的线程工作 即:开启线程池工作
    for(int i = 0;i < m_threadSum;i++)
    {
        Thread::ptr t(new Thread);
        t->start();
        m_threads.push_back(t);
    }

    g_logger->info("threads start");

    return 0;
}

//开启服务器 事件循环
void TcpServer::start()
{
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

    event_base_free(m_base);
    
}


//事件回调函数 负责处理新连接
void TcpServer::StartAccept(evutil_socket_t sock_fd, short what, void *arg)
{
    // 接受来自客户端 ip port 信息的 socket_in
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));

    socklen_t ac_len = sizeof(struct sockaddr_in);
    evutil_socket_t ac_fd = accept(sock_fd, (struct sockaddr *)&addr, &ac_len);
    if(ac_fd < 0)
    {
        g_logger->error("accept client error! errno=%d, is:%s", errno, strerror(errno));
        exit(-1);
    }

   //去线程池中 选择一个线程 去处理客户端的数据收发 在这时:需要一个下标来指示需要选择的线程
   //此处是回调函数 转到普通函数中 去做
    TcpServer* s = (TcpServer *)arg;
    s->handleClient(ac_fd, std::string(inet_ntoa(addr.sin_addr)), (int)ntohs(addr.sin_port));

}


//处理客户端数据收发
void TcpServer::handleClient(evutil_socket_t sock_fd, const std::string& ip, int port)
{
    /*-------------简化操作 选择轮询的方式 去选择线程----------*/
    
    //从每个线程中 取出各自的内核表
    auto child = m_threads[next_thread_index];
    struct event_base *sub_base = child->getBase();
    //取完内核表 轮询到下一个线程  简易的负载均衡 平均分配
    next_thread_index = (next_thread_index + 1) % m_threadSum;

    //创建 bufferevent缓冲区负责数据收发 与当前线程的event_base绑定 与当前通信套接字绑定
    //指定释放bufferevent时 自动关闭与之绑定的通信套接字
    struct bufferevent* bev = bufferevent_socket_new(sub_base, sock_fd, BEV_OPT_CLOSE_ON_FREE);
    if(!bev)
    {
        g_logger->error("bufferevent create error");
        event_base_loopbreak(sub_base);
        exit(-1);
    }


    /* 封装一个通信类 来完成数据收发*/
    TcpSocket* s = new TcpSocket(this, bev, ip, port);

    // 设置 缓冲区回调函数 从 通信类 TcpSocket 中调用
    bufferevent_setcb(bev, s->BufferReadCB, s->BufferWriteCB, s->BufferEventCB, s);

    //将缓冲区 使能化 可读 可写 可接受信号
    bufferevent_enable(bev, EV_READ | EV_WRITE | EV_SIGNAL);

    ++m_conSum;
    if(m_conSum % 999 == 0)
    {
        gettimeofday(&ed_tv, nullptr);
        int64_t t = (ed_tv.tv_sec - be_tv.tv_sec) * 1000 + (ed_tv.tv_usec - be_tv.tv_usec) / 1000;        g_logger->info("finish connections={}, used time={}", m_conSum, t);
        memcpy(&be_tv, &ed_tv, sizeof(struct timeval));
    }
    //处理客户端连接 事件 小难点！ c++ 多态 隐形体现
    // TcpServer类作为一个基类 肯定要被继承 执行到该语句根据开发人员重写的conncect_event去执行三
    conncectEvent(s);

}
}





