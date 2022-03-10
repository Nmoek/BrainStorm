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
     * @brief TCP通信类默认构造函数
     */
    TcpSocket();

    /**
     * @brief TCP通信类构造函数
     * @param[in] server 服务器指针
     * @param[in] bev libevent缓存区
     * @param[in] ip 客户端IP地址
     * @param[in] port 客户端端口
     */
    TcpSocket(struct bufferevent* bev, const std::string& ip, uint16_t port, int fd);

    /**
     * @brief TCP通信类析构函数
     */
    ~TcpSocket();

    /**
     * @brief 获取客户端IP地址
     * @return std::string 
     */
    std::string getClientIp() const {return m_ip;}

    /**
     * @brief 获取客户端端口
     * @return int 
     */
    uint16_t getClientPort() const {return m_port;}

    /**
     * @brief 从客户端读取数据
     * @param[out] data 接收数据的缓冲区
     * @param[in] size 期望接收数据大小 字节
     */
    void readData(void *data, int size);

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
    void setUserName(std::string name) {m_userName = name;};

    /**
     * @brief 获取套接字上对应的用户名
     * @return std::string 
     */
    std::string getUserName() const {return m_userName;};


    /**
     * @brief 设置套接字文件句柄
     * @param[in] fd 
     */
    void setFd(int fd) {m_fd = fd;}

    /**
     * @brief 获取套接字文件句柄
     * @return int 
     */
    int getFd() const {return m_fd;}

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
    /// 与客户端通信的 bufferevent 句柄
    struct bufferevent* m_bev;
    /// 客户端ip地址
    std::string m_ip;
    /// 客户端 端口号
    uint16_t m_port;
    /// 与客户端 通信套接字绑定的 用户名称
    std::string m_userName;
    /// 文件句柄
    int m_fd;

};
}


#endif // TCPSOCKET_H
