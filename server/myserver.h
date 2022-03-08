#ifndef _MYSERVER_H_
#define _MYSERVER_H_

#include "tcpserver.h"
#include "db.h"
#include "common.h"
#include "user.h"
#include "tcpsocket.h"

//#include <spdlog/spdlog.h>
//#include <spdlog/sinks/stdout_color_sinks.h>
//#include <spdlog/cfg/env.h>
//#include <spdlog/sinks/rotating_file_sink.h>

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>


#include <string>
#include <map>
#include <memory>


#define DEBUG

#define QUESTION_NUM 5

namespace BrainStorm
{

/**
 * @brief 具体服务器类
 */
class MyServer : public TcpServer
{
public:
    typedef std::shared_ptr<MyServer> ptr;

    /**
     * @brief 具体服务器类构造函数
     * @param[in] threadSum 起始线程数量 
     */
    MyServer(int threadSum = 10);

protected:
    /**
     * @brief 新连接事件回调
     */
    void conncectEvent(TcpSocket * ) override;

    /**
     * @brief 读事件回调
     */
    void readEvent(TcpSocket *) override;

    /**
     * @brief 写事件回调
     */
    void writeEvent(TcpSocket *) override;

    /**
     * @brief 连接关闭事件回调
     */
    void closeEvent(TcpSocket *, short) override;

private:
    /**
     * @brief 初始化rank分对照表
     */
    void initRankTable();

    /**
     * @brief 向客户端写数据
     * @param[in] s 套接字对象
     * @param[in] inJson 封装好的Json数据
     */
    void writeData(TcpSocket * s, const Json::Value &inJson);

    /**
     * @brief 处理用户注册
     * @param[in] s 套接字对象
     * @param[in] inJson 封装好的Json数据
     */
    void dealRegister(TcpSocket * s, const Json::Value &inJson);

    /**
     * @brief 处理用户登陆
     * @param[in] s 套接字对象
     * @param[in] inJson 封装好的Json数据
     */
    void dealLogin(TcpSocket * s, const Json::Value &inJson);

    /**
     * @brief 获取单机模式的题目
     * @param[in] s 套接字对象
     */
    void singleGetQustion(TcpSocket * s);

    /**
     * @brief 处理用户申请匹配
     * @param[in] s 套接字对象
     */
    void dealCompete(TcpSocket * s);

    /**
     * @brief 处理用户匹配对决 
     * @param[in] first 玩家1套接字对象
     * @param[in] second 玩家2套接字对象
     */
    void startCompete(TcpSocket * first, TcpSocket * second);

    /**
     * @brief 处理答题转发
     * @param[in] s 套接字对象
     * @param[in] inJson 封装好的Json数据
     */
    void dealAnswer(TcpSocket * s, const Json::Value &inJson);

    /**
     * @brief 处理答题结束
     * @param[in] s 套接字对象
     * @param[in] inJson 封装好的Json数据  
     */
    void dealCompeteResult(TcpSocket * s, const Json::Value &inJson);


private:
    //在线用户列表 key:用户名 value:用户指针
    std::map<std::string, User::ptr> m_onlineUsers;
    //rank 对照表   key值:rank分  value值:对应的分段说明
    std::map<int, std::string> rank_map;
    //等待 匹配队列
    std::map<int, TcpSocket *> m_competors;
    /// 数据库句柄
    MySqlConnection::ptr m_db;
    // 在线用户列表 互斥锁
    std::mutex online_user_mutex;
    /// 等待匹配队列 互斥锁
    std::mutex compete_mutex;
};
}

#endif // MYSERVER_H
