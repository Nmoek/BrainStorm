#ifndef _THREAD_H_
#define _THREAD_H_

#include "noncopyable.h"
#include "tcpsocket.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory>
#include <functional>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/buffer_compat.h>
#include <event2/event_struct.h>
#include <signal.h>
#include <map>
#include <mutex>

namespace BrainStorm
{

class MyServer;

/**
 * @brief 线程类
 */
class Thread: public std::enable_shared_from_this<Thread>, Noncopyable 
{
    friend class MyServer;
public:
    typedef std::shared_ptr<Thread> ptr;

    /**
     * @brief 线程构造函数
     */
    Thread(TcpServer *server);

    /**
     * @brief 线程析构函数
     */
    ~Thread();

    /**
     * @brief 线程开启
     */
    void start();

    /**
     * @brief 获取libevent句柄指针
     * @return struct event_base* 
     */
    struct event_base* getBase() const  {return m_base;}

    /**
     * @brief 添加客户端连接
     * @param[in] s 
     */
    void addClint(int fd, TcpSocket::ptr s);

    /**
     * @brief 根据套接字fd删除客户端连接
     * @param[in] fd 
     * @return true 删除成功
     * @return false 删除失败
     */
    bool delClient(int fd);

    /**
     * @brief 获取客户端连接
     * @param[in] fd 
     * @return TcpSocket::ptr 
     */
    TcpSocket::ptr getClient(int fd) const {return m_clients.find(fd) == m_clients.end() ? nullptr : m_clients.find(fd)->second;}

    /**
     * @brief 获取管道写端
     * @return int 
     */
    int getPipeWrite() const {return m_pipe[1];}
    
public:
    /**
     * @brief 获取当前运行线程指针
     */
    static Thread* GetThis();

    /**
     * @brief 获取当前线程所属的服务器指针
     * @return TcpServer* 
     */
    static TcpServer* GetServer();

protected:
    /**
     * @brief 线程指定回调函数
     * @return void* 
     */
    static void* ThreadCallBack(void *);

    /**
     * @brief libevent事件回调，负责处理管道读端
     * @param[in] m_bev 缓冲区指针
     * @param[in] ctx 程序上下文 
     */
    static void PipeReadCB(struct bufferevent *m_bev, void* ctx);

    /**
     * @brief 线程执行函数
     */
    void run();

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
    /// libevent句柄
    struct event_base* m_base;
    /// 线程ID
    pthread_t m_threadId;
    /// 管道描述符
    int m_pipe[2];
    /// 与管道绑定的缓冲区
    struct bufferevent* m_pipeBev;
    /// 客户端队列
    std::map<int, TcpSocket::ptr> m_clients;
    /// 客户端队列的锁
    std::mutex m_mutex;
    /// 线程所属服务器指针
    TcpServer * m_server;

};
}

#endif // THREAD_H
