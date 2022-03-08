#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_

#include "noncopyable.h"
#include <string>
#include <memory>
#include <event2/bufferevent.h>

namespace BrainStorm
{

class TcpServer;

/**
 * @brief TCP通信类
 * @details 负责和客户端建立通信
 */
class TcpSocket   
{
public:
    typedef std::shared_ptr<TcpSocket> ptr;

    /**
     * @brief TCP通信类构造函数
     * @param[in] server 服务器指针
     * @param[in] bev libevent缓存区
     * @param[in] ip 客户端IP地址
     * @param[in] port 客户端端口
     */
    TcpSocket(TcpServer* server, struct bufferevent* bev, const std::string& ip, int port);

    /**
     * @brief TCP通信类析构函数
     */
    ~TcpSocket();

    /**
     * @brief 获取客户端IP地址
     * @return std::string 
     */
    std::string getClientIp();

    /**
     * @brief 获取客户端端口
     * @return int 
     */
    int getClientPort();

    /**
     * @brief 从客户端读取数据
     * @param[out] data 接收数据的缓冲区
     * @param[in] size 期望接收数据大小 字节
     */
    int readData(void *data, int size);

    /**
     * @brief 从客户端写入数据
     * @param[in] data 写入数据的缓冲区
     * @param[in] size 期望写入数据大小 字节
     */
    void writeData(const void *data, int size);

    /**
     * @brief 将套接字和游戏用户名绑定
     * @param name 
     */
    void setUserName(std::string name);

    /**
     * @brief 获取套接字上对应的用户名
     * @return std::string 
     */
    std::string getUserName();

public:
    /**
     * @brief libevent缓存区读事件回调函数
     * @param[in] m_bev libevent缓存区指针
     * @param[in] ctx 其他参数
     */
    static void BufferReadCB(struct bufferevent *m_bev, void *ctx);

    /**
     * @brief libevent缓存区写事件回调函数
     * @param[in] m_bev libevent缓存区指针
     * @param[in] ctx 其他参数
     */
    static void BufferWriteCB(struct bufferevent *m_bev, void *ctx);

    /**
     * @brief libevent缓存区其他事件回调函数
     * @param[in] m_bev libevent缓存区指针
     * @param[in] ctx 其他参数
     */
    static void BufferEventCB(struct bufferevent *m_bev, short what, void *ctx);

private:
    /// 当前服务器 指针
    static TcpServer* m_server;
    /// 与客户端通信的 bufferevent 句柄
    bufferevent* m_bev;
    /// 客户端ip地址
    std::string m_ip;
    /// 客户端 端口号
    int m_port;
    /// 与客户端 通信套接字绑定的 用户名称
    std::string m_userName;

};
}


#endif // TCPSOCKET_H
