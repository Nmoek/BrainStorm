#include "tcpsocket.h"
#include "myserver.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>

namespace BrainStorm
{


static std::shared_ptr<spdlog::logger> g_logger = spdlog::get("BrainStorm");


TcpSocket::TcpSocket()
    :m_bev(nullptr)
    ,m_ip("")
    ,m_port(-1)
    ,m_fd(-1)
{
    
}

TcpSocket::TcpSocket(bufferevent* bev, const std::string& ip, uint16_t port, int fd)
    :m_bev(bev)
    ,m_ip(ip)
    ,m_port(port)
    ,m_fd(fd)
{

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
    Thread::GetServer()->readEvent(s);

}

//缓冲区回调函数 检测事件 表面函数 实际转回到 服务器的类中去处理
void TcpSocket::BufferEventCB(struct bufferevent *m_bev, short what, void *ctx)
{
    TcpSocket* s = (TcpSocket*)ctx;
    Thread::GetServer()->closeEvent(s, what);
}


// 通过bufferevent 读取数据  相当于 recv
void TcpSocket::readData(void *data, int size)
{
    bufferevent_read(m_bev, data, size);

}

//通过 bufferevent 写入数据  相当于send
void TcpSocket::writeData(const void *data, int size)
{
    bufferevent_write(m_bev, data, size);
}


}
