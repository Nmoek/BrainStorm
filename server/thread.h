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

namespace BrainStorm
{

/**
 * @brief 线程类
 */
class Thread: public std::enable_shared_from_this<Thread>, Noncopyable   //不可拷贝
{
public:
    typedef std::shared_ptr<Thread> ptr;

    /**
     * @brief 线程构造函数
     */
    Thread();

    /**
     * @brief 线程开启
     */
    void start();

    /**
     * @brief 获取libevent句柄指针
     * @return struct event_base* 
     */
    struct event_base* getBase() const  {return m_base;}


protected:
    /**
     * @brief 线程指定回调函数
     * @return void* 
     */
    static void* ThreadCallBack(void *);

     /**
      * @brief libevent事件回调，负责处理管道读端
      * @param[in] fd libevent套接字句柄
      * @param[in] what 错误代码
      * @param[in] arg 其他参数
      */
    static void PipeReadCB(evutil_socket_t fd, short what, void *arg);

    /**
     * @brief 线程执行函数
     */
    void run();

private:
    /// libevent句柄
    struct event_base * m_base;
    /// 线程ID
    pthread_t m_threadId;
    /// 管道描述符
    int m_pipe[2];
    /// 与管道绑定的 事件
    struct event* m_pipeEvent;

};
}

#endif // THREAD_H
