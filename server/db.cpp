#include "db.h"

namespace BrainStorm
{

static std::shared_ptr<spdlog::logger> g_logger = spdlog::get("BrainStorm");


MySqlConnection::MySqlConnection(const std::string& host, const std::string& user_name, const std::string& password, const std::string& baseName)
{
    mysql_library_init;
    //初始化 数据库的句柄
    m_con = mysql_init(nullptr);
    if(!m_con)  // 如果初始化 失败
    {
        g_logger->error("mysql init error :{}", mysql_error(m_con));
        exit(-1);
    }

    //连接 mysql服务器
    if(!mysql_real_connect(m_con, host.c_str(), user_name.c_str(), password.c_str(), baseName.c_str(), 0, NULL, 0))
    {
        g_logger->error("mysql connect error :{}", mysql_error(m_con));
        exit(-1);
    }


    //设置mysql 字符集 中文乱码问题 query查询
    int ret = mysql_query(m_con, "set names utf8");
    if(ret != 0) //成功返回0
    {
        g_logger->error("set names error: {}", mysql_error(m_con));
        exit(-1);

    }


}

MySqlConnection::~MySqlConnection()
{
    if(m_con)
    {
        mysql_close(m_con);
        mysql_library_end;
    }
}


// 数据库 常用操作 语句
bool MySqlConnection::exec(const char *sql)
{
    // 执行操作之前 上锁 因为 数据库在多线程环境下被操作
    std::unique_lock<std::mutex> m_lock(m_mutex);

    int ret = mysql_query(m_con, sql);
    if(ret != 0)
    {
        g_logger->error("mysql exec error: {}", mysql_error(m_con));
        return false;
    }

    return true;

}


// 数据库 查询 并且将 结果至于 传入的 Jsaon中
bool MySqlConnection::query(const char *sql, Json::Value &value)
{
    // 执行操作之前 上锁 因为 数据库在多线程环境下被操作
    std::unique_lock<std::mutex> m_lock(m_mutex);

    //执行完成之后 这个语句并不返回结果 结果还在mysql服务器中
    int ret = mysql_query(m_con, sql);
    if(ret != 0)
    {
        g_logger->error("mysql query error: {}", mysql_error(m_con));
        return false;
    }

    //从 mysql服务器下载 查询到的结果
    MYSQL_RES *res = mysql_store_result(m_con);  //查询结果集
    if(res == NULL)
    {
        if(mysql_error(m_con) == 0)
        {
            g_logger->info("select noting");
            return true;
        }
        else
        {
            g_logger->error("select error: {}", mysql_error(m_con));
            return false;
        }
    }

    /*以行为单位 从结果集 获取数据*/
    //获取 总的列数  即:字段数
    unsigned int num_fields = mysql_num_fields(res);

    //获取表头
    MYSQL_FIELD *fetch_field = mysql_fetch_field(res);

    MYSQL_ROW row;
    while((row = mysql_fetch_row(res))) //一行行循环 获取数据
    {
        // 添加 json数据结构
        for(unsigned int i = 0;i < num_fields;i++)
        {
            value[fetch_field[i].name].append(row[i]);
        }

    }

     // 释放 查询结果 集合
    mysql_free_result(res);

    return true;


}
}
