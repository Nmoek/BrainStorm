#include "user.h"

namespace BrainStorm
{

User::User(const std::string& user_name, const std::string& user_passwd, const int rank, TcpSocket * s)
    :m_userName(user_name)
    ,m_userPass(user_passwd)
    ,m_rank(rank)
    ,m_socket(s)
{

}

User::~User()
{

}

// 获取 用户rank 分
int User::getRank() const
{
    return m_rank;

}

std::string User::getName() const 
{
    return m_userName;

}

TcpSocket * User::getSocket() const
{
    return m_socket;

}

void User::changeRank(int temp)
{
    if(temp <= 0)
        m_rank = 0;
    else
        m_rank = temp;
}



}
