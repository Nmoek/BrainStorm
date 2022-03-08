#ifndef _USER_H_
#define _USER_H_

#include "tcpsocket.h"
#include <string>
#include <memory>

namespace BrainStorm
{

/**
 * @brief 用户类
 */
class User
{
public:
    typedef std::shared_ptr<User> ptr;

    /**
     * @brief 用户类构造函数
     * @param[in] user_name 用户名
     * @param[in] user_passwd 用户密码
     * @param[in] rank 用户rank分
     * @param[in] s 套接字对象智能指针
     */
    User(const std::string& user_name, const std::string& user_passwd, const int rank, TcpSocket * s);

    /**
     * @brief 用户类析构函数
     */
    ~User();

    /**
     * @brief 获取用户rank分
     * @return int 
     */
    int getRank() const;

    /**
     * @brief 获取用户名
     * @return std::string 
     */
    std::string getName() const;

    /**
     * @brief 获取用户对应的套接字对象
     * @return TcpSocket * 
     */
    TcpSocket * getSocket() const;

    /**
     * @brief 修改用户rank分
     */
    void changeRank(int);


private:
    /// 用户名
    std::string m_userName;
    /// 用户 密码
    std::string m_userPass;
    /// 用户rank分数
    int m_rank;
    //一个用户 对应 一个socket与服务器进行通信
    TcpSocket * m_socket;
};
}

#endif // USER_H
