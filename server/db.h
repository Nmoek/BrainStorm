#ifndef DB_H
#define DB_H

#include <stdio.h>
#include <mysql/mysql.h>
#include <mutex>
#include <string.h>
#include <error.h>
#include <jsoncpp/json/json.h>
#include <spdlog/spdlog.h>
#include <memory>

#include "noncopyable.h"

namespace BrainStorm
{

/**
 * @brief MySql连接类
 */
class MySqlConnection: Noncopyable
{
public:
    typedef std::shared_ptr<MySqlConnection> ptr;
    /**
     * @brief MySql连接类构造函数
     * @param[in] host 主机域名/IP地址
     * @param[in] user_name 用户名
     * @param[in] password 密码
     * @param[in] baseName 数据库名称
     */
    MySqlConnection(const std::string& host, const std::string& user_name, const std::string& password, const std::string& baseName);

    /**
     * @brief MySql连接类析构函数
     */
    ~MySqlConnection();

    /**
     * @brief 数据库执行语句,不需要返回结果
     * @param[in] sql sql语句
     * @return true 执行成功
     * @return false 执行失败
     */
    bool exec(const char *sql);

    /**
     * @brief 数据库查询语句，将结果json中
     * @param[in] sql sql语句
     * @param[out] value 查询结果 
     * @return true 查询成功
     * @return false 查询失败
     */
    bool query(const char *sql, Json::Value &value);

private:
    /// mysql句柄
    MYSQL *m_con;
    /// 互斥锁
    std::mutex m_mutex;
};
}
#endif // DB_H
