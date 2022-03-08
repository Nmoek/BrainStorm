#include "tcpsocket.h"
#include "tcpserver.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace BrainStorm
{

TcpServer* TcpSocket::m_server = nullptr;
static std::shared_ptr<spdlog::logger> g_logger = spdlog::get("BrainStorm");


TcpSocket::TcpSocket(TcpServer* server, bufferevent* bev, const std::string& ip, int port)
    :m_bev(bev)
    ,m_ip(ip)
    ,m_port(port)
{
    m_server = server;

}

TcpSocket::~TcpSocket()
{
    if(m_bev)
        bufferevent_free(m_bev);
    
}

//缓冲区回调函数 可读事件 表面函数 实际转回到 服务器的类中去处理
void TcpSocket::BufferReadCB(struct bufferevent *m_bev, void* ctx)
{
    TcpSocket* s = (TcpSocket*)ctx;
    m_server->readEvent(s);

}

//缓冲区回调函数 可写事件 表面函数 实际转回到 服务器的类中去处理
void TcpSocket::BufferWriteCB(struct bufferevent *m_bev, void *ctx)
{
    TcpSocket* s = (TcpSocket*)ctx;
    m_server->writeEvent(s);
}

//缓冲区回调函数 检测事件 表面函数 实际转回到 服务器的类中去处理
void TcpSocket::BufferEventCB(struct bufferevent *m_bev, short what, void *ctx)
{
    TcpSocket* s = (TcpSocket*)ctx;
    m_server->closeEvent(s, what);
}


// 获取客户端 m_ip
std::string TcpSocket::getClientIp()
{
    return m_ip;
}

//获取客户端 m_port
int TcpSocket::getClientPort()
{
    return m_port;
}


// 通过bufferevent 读取数据  相当于 recv
int TcpSocket::readData(void *data, int size)
{
    return bufferevent_read(m_bev, data, size);

}

//通过 bufferevent 写入数据  相当于send
void TcpSocket::writeData(const void *data, int size)
{
    bufferevent_write(m_bev, data, size);
}


// 设置 用户名  相当于将 用户名和当前通信套接字 绑定
void TcpSocket::setUserName(std::string name)
{
    m_userName = name;
}


//获取用户名
std::string TcpSocket::getUserName()
{
    return m_userName;
}
}
