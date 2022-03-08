#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_


#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>

#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>
#include <event2/bufferevent_compat.h>
#include <time.h>

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
    TcpServer(int tread_num = 10);

    /**
     * @brief 监听
     * @param[in] port 监听端口
     * @param[in] ip 监听IP地址
     * @return int 
     */
    int listen(int port, const std::string &ip = "");

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
    void handleClient(evutil_socket_t sock_fd, const std::string & ip, int port);


protected:
    /**
     * @brief 事件回调函数 负责处理新连接
     */
    static void StartAccept(evutil_socket_t, short, void*);

    virtual void conncectEvent(TcpSocket *) = 0;
    virtual void readEvent(TcpSocket *) = 0;
    virtual void writeEvent(TcpSocket *) = 0;
    virtual void closeEvent(TcpSocket *, short) = 0;

private:
    //线程池
    std::vector<Thread::ptr> m_threads;
    //线程池中存在的总线程数
    int m_threadSum;
    // libevent句柄
    event_base * m_base;
    // 轮询中 下一个线程的 下标
    int next_thread_index;


    uint64_t m_conSum = 0;
    struct timeval be_tv;
    struct timeval ed_tv;


};
}

#endif // TCPSERVER_H
