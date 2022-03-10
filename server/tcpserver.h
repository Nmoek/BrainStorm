#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <functional>

#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>
#include <event2/bufferevent_compat.h>

#include "tcpsocket.h"
#include "thread.h"
#include "noncopyable.h"


namespace BrainStorm
{

class TcpSocket;

/**
 * @brief TCP服务器类基类
 */
class TcpServer: std::enable_shared_from_this<TcpServer>, Noncopyable
{
    // 由于 虚函数在 保护权限中 将TcpSocket 声明为友元 方便调用
    friend class TcpSocket;
public:
    typedef std::shared_ptr<TcpServer> ptr;
    
    /**
     * @brief TCP服务器类构造函数
     * @param[in] tread_num 起始线程数 
     */
    TcpServer(uint64_t tread_num = 10);

    /**
     * @brief 监听
     * @param[in] port 监听端口
     * @param[in] ip 监听IP地址
     * @return int 
     */
    int listen(uint16_t port, const std::string &ip = "");

    /**
     * @brief 开启服务器
     */
    void start();

    /**
     * @brief 处理客户端数据收发
     * @param[in] sock_fd libevent套接字句柄
     * @param[in] ip 客户端IP地址
     * @param[in] port 客户端端口
     */
    void handleClient(evutil_socket_t sock_fd, const std::string & ip, uint16_t port);

    /**
     * @brief 获取当前线程运行的服务器指针
     * @return TcpServer* 
     */
    static TcpServer* GetThis();

protected:
    /**
     * @brief 事件回调函数 负责处理新连接
     * @param[in] sock_fd 和事件绑定的文件句柄
     * @param[in] what 传回的事件
     * @param[in] arg 其余参数（传递上下文）
     */
    static void StartAccept(evutil_socket_t sock_fd, short what, void *arg);

    void setThis();

    virtual void conncectEvent(TcpSocket *) = 0;
    virtual void readEvent(TcpSocket *) = 0;
    virtual void writeEvent(TcpSocket *) = 0;
    virtual void closeEvent(TcpSocket *, short) = 0;

private:
    /**
     * @brief 给子线程传递的连接参数结构体
     */
    struct SockItem
    {
        typedef std::shared_ptr<SockItem> ptr;
        int ac_fd;
        std::string ip;
        uint16_t port;
    };


private:
    /// 线程池
    std::vector<Thread::ptr> m_threads;
    /// 线程池中存在的总线程数
    uint64_t m_threadSum;
    /// libevent句柄
    struct event_base* m_base;
    /// 监听事件
    struct event* m_listenEv;
    // 轮询中 下一个线程的 下标
    size_t m_nextThread;

};
}

#endif // TCPSERVER_H
