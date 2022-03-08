#include "myserver.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <string>

namespace BrainStorm
{
struct LogIniter
{
    LogIniter()
    {
    #ifdef DEBUG
        auto g_logger = spdlog::stdout_color_mt("BrainStorm");
        // g_logger->set_level(spdlog::level::debug);
    #else
        auto g_logger = spdlog::rotating_logger_mt("BrainStorm", "./BrainStorm", 1024*1024*5, 3);
        g_logger->flush_on(spdlog::level::info);
    #endif

        g_logger->set_pattern("[%^%l%$]\t\t[%Y-%m-%d %H:%M:%S]\t%t\t[%n]:%v");

    }
};
static LogIniter _initer;
static auto g_logger = spdlog::get("BrainStorm");

MyServer::MyServer(int threadSum)
    :TcpServer(threadSum)
{
    //spdlog 日志 设置
    g_logger->info("Welcome to BrainStorm server v1.0.0  !");

    //初始化 数据库
    m_db.reset(new MySqlConnection("192.168.77.136", "root", "123456", "BrainStorm"));


    // 初始化段位表
    initRankTable();

}


/*------------------------------------------虚函数重写------------------------------------------*/
// 客户端连接事件
void MyServer::conncectEvent(TcpSocket * s)
{
    g_logger->debug("new connction: [{} : {}]", s->getClientIp(), s->getClientPort());
}


//读取数据事件
void MyServer::readEvent(TcpSocket * s)
{
    //读缓冲区
    char buf[1024] = {0};

    //循环 一直读取来自客户端的数据
    while(1)
    {
        
        /*先读长度 再读数据*/
        int len = 0;
        s->readData(&len, sizeof(len));

        if(len <= 0)   //如果 长度小于等于0跳出循环
            break;

        s->readData(buf, 5);
 
    }



    //数据解析  将读取的数据解析为Json格式
    Json::Value root;
    Json::Reader reader; //json 解析器

    if(!reader.parse(buf, root)) //解析不正确 就退出
    {
        g_logger->error("json parse error!");
        return;
    }

    //将要执行的 操作提取出来
    int cmd = root["cmd"].asInt();

    switch (cmd)
    {
    case REG: dealRegister(s, root); break;

    case LOG: dealLogin(s, root); break;

    case SINGLEGETQUESTION: singleGetQustion(s); break;

    case COMPETE: dealCompete(s); break;

    case ANSWER: dealAnswer(s, root); break;

    case COMPETERESULT: dealCompeteResult(s, root); break;

    default:
    {
        g_logger->error("invalid command!");
        closeEvent(s, 0);
    }break;

    }
    
}


// 写入数据事件
void MyServer::writeEvent(TcpSocket * s) 
{

}


//客户端 关闭事件
void MyServer::closeEvent(TcpSocket * s, short what) 
{
    //客户端 退出的 时候 应该清除 相关容器(用户在线列表 匹配等待列表)中的 信息
    std::unique_lock<std::mutex> lock(online_user_mutex);

    auto online_it = m_onlineUsers.find(s->getUserName());
    if(online_it != m_onlineUsers.end())
    {
        m_onlineUsers.erase(online_it);
        lock.unlock();

        g_logger->info("user :{} offline", s->getUserName());

        std::unique_lock<std::mutex> compete_lock(compete_mutex);
        auto compete_it = m_competors.find(online_it->second->getRank());
        if(compete_it != m_competors.end())
        {
            g_logger->info("user: {} erase from m_competors", compete_it->second->getUserName());
            m_competors.erase(compete_it);
        }
    }
   

    g_logger->debug("client closed[{} : {}]", s->getClientIp(), s->getClientPort());

    delete s;

    

}


/*------------------------------------------------------------------------------*/

//向客户端 写入数据
void MyServer::writeData(TcpSocket * s, const Json::Value &inJson)
{
    std::string data = inJson.toStyledString();
    s->writeData(data.c_str(), data.length());
}


// 处理 注册
void MyServer::dealRegister(TcpSocket * s, const Json::Value &inJson)
{
    std::string user = inJson["user"].asString();
    std::string pass = inJson["passwd"].asString();

    char sql[100] = {0};
    sprintf(sql, "select * from user where name='%s' and pass='%s'", user.c_str(), pass.c_str());

    int result = OK;
    Json::Value outJson;

    //调用 数据库查询
    if(!(m_db->query(sql, outJson)))
    {
        result = ERROR;
        g_logger->error("mysql query error\n");
    }

    // 用户是否已经注册过
    if(outJson.size())
    {
        result = USEREXIST;
    }
    else
    {
        // 调用 普通sql 语句执行函数即可
        sprintf(sql, "INSERT INTO user VALUES('%s','%s',0)", user.c_str(), pass.c_str());
        if(!m_db->exec(sql))
        {
            result = ERROR;
            g_logger->error("table insert error\n");

        }
        else
        {
            g_logger->info("[{}:{}] register sucess, user_name={}", s->getClientIp(), 
                s->getClientPort(), user);

        }
    }

    //配置json 包
    outJson["cmd"] = REG;
    outJson["result"] = result;

    writeData(s, outJson);

}


//处理 登录
void MyServer::dealLogin(TcpSocket * s, const Json::Value &inJson)
{
    std::string user = inJson["user"].asString();
    std::string pass = inJson["passwd"].asString();

    char sql[100] = {0};
    sprintf(sql, "select * from user where name='%s' and pass='%s'", user.c_str(), pass.c_str());


    int result = OK;  // 默认 OK
    int rank = 0;
    Json::Value json;


    if(!m_db->query(sql, json))
    {
        g_logger->error("login select error, user no registe\n");
        result = ERROR;
    }

     //给在线列表加锁
    std::unique_lock<std::mutex> lock(online_user_mutex);

    // 用户存在于数据库 已经注册完成
    if(json.isMember("name"))
    {
        if(m_onlineUsers.find(user) != m_onlineUsers.end())
        {
            result = USERLOGREADY;
        }
        else
        {
            rank = atoi(json["rank"][0].asString().c_str());
            User::ptr log_user(new User(user, pass, rank, s));

            //将 登陆成功的用户  插入用户在线列表
            m_onlineUsers.insert({user, log_user});

            g_logger->info("用户{} [{}:{}]:login", user, s->getClientIp(), s->getClientPort());

            s->setUserName(user);
        }
    }
    else
    {
        result = NAMEORPASS;
    }
 
    json.clear();
    json["cmd"] = LOG;
    json["result"] = result;
    json["user"] = user;
    json["rank_explain"] = rank_map[rank];

    writeData(s, json);

}


// 获取 单机模式的题目
void MyServer::singleGetQustion(TcpSocket * s)
{
    char sql[100] = {0};
    sprintf(sql, "select * from question order by rand() limit %d", QUESTION_NUM);

    int result = OK;
    Json::Value outJson;
    
    if(!(m_db->query(sql, outJson)) || outJson["question"].size() != QUESTION_NUM)
    {
        g_logger->error("single select question error\n");
        result = ERROR;
    }

    Json::Value json;
    json["cmd"] = SINGLEGETQUESTION;
    json["result"] = result;
    json["question"] = outJson;

    g_logger->info("用户{} [{}:{}] geting questions:{}\n", s->getUserName(), s->getClientIp(), s->getClientPort(),json.toStyledString());

    writeData(s, json);

}


//初始化 rank 对照表
void MyServer::initRankTable()
{
    char buf[255] = {0};
    int rank = 0;
    int num = 0;
    for(int i = 0;i < 100;i++)
    {
        if(i < 9)
        {
            rank = i / 3;
            num = i % 3;
            sprintf(buf, "知识爱好者%d %d颗星", 3 - rank, num + 1);
        }
        else if(i >= 9 && i < 18)
        {
            rank = (i - 9) / 3;
            num = (i - 9) % 3;
            sprintf(buf, "问答达人%d %d颗星", 3- rank, num + 1);

        }
        else if(i >= 18 && i < 34)
        {
            rank = (i - 18) / 4;
            num = (i - 18) % 4;
            sprintf(buf, "学者%d %d颗星", 4 - rank, num + 1);

        }
        else if(i >= 34 && i < 50)
        {
            rank = (i- 34) / 4;
            num = (i - 34) % 4;
            sprintf(buf, "资深学者%d %d颗星", 4 - rank, num + 1);

        }
        else if(i >= 50 && i < 75)
        {
            rank = (i - 50) / 5;
            num = (i - 50) % 5;
            sprintf(buf, "专家%d %d颗星", 5 - rank, num + 1);

        }
        else if(i >= 75 && i < 100)
        {
            rank = (i - 75) / 5;
            num = (i - 75) % 5;
            sprintf(buf, "资深专家%d %d颗星", 5 - rank, num + 1);

        }

        rank_map.insert(std::make_pair(i, std::string(buf)));
    }

}


//处理 申请匹配 函数
void MyServer::dealCompete(TcpSocket * s)
{
    //给 等待匹配队列 上锁
    std::unique_lock<std::mutex> m_lock(compete_mutex);

    // 对手的 套接字
    TcpSocket * opp = nullptr;

    //获取当前 用户的rank分 通过 用户在线列表进行查找
    int rank = m_onlineUsers[s->getUserName()]->getRank();


    /*---------------------整个匹配游戏核心 匹配算法----------------*/
    //等待匹配队列 是否为空
    if(!m_competors.size())  //为空当前用户 直接加入
    {
        m_competors.insert({rank, s});
    }
    else  //不为空
    {
        //去 等待匹配队列中 查找是否 存在同一段位的人
        auto it = m_competors.find(rank);
        if(it != m_competors.end())  //存在同段位
        {
            opp = it->second;
            m_competors.erase(it);

        }
        else   //不存在同一段位  寻找其他段位的人  +-5
        {
            for(int i = 0;i <= 5;i++)
            {
                //先找 rank分高的
                it = m_competors.find(rank + i);
                if(it != m_competors.end())
                {
                    opp = it->second;
                    m_competors.erase(it);
                    break;

                }
                else
                {
                    //再找rank分低的
                    it = m_competors.find(rank - i);
                    if(it != m_competors.end())
                    {
                        opp = it->second;
                        m_competors.erase(it);
                        break;

                    }

                }
            }

        }
    }




    // 如果没有找到 合适的对手 加入到匹配队列中
    if(!opp)
    {
        m_competors.insert({rank, s});

    }
    else  //找到对手 开始答题对决
    {
        startCompete(s, opp);
    }


}


// 处理 匹配对决 函数
void MyServer::startCompete(TcpSocket * first, TcpSocket * second)
{
    //去数据库中 查询获取 题目数据
    char sql[100] = {0};
    sprintf(sql, "select *from question order by rand() limit %d", QUESTION_NUM);

    int result = OK;
    Json::Value outJson;

    if(!m_db->query(sql, outJson))
    {
        g_logger->error("compete select question error");
        result = ERROR;
    }


    //配置 发送给 自己的 json包
    Json::Value self_json;
    self_json["cmd"] = COMPETEGETQUESTION;
    self_json["result"] = result;
    self_json["question"] = outJson;
    self_json["opp_userName"] = second->getUserName();
    self_json["opp_rank"] = rank_map[m_onlineUsers[second->getUserName()]->getRank()];
    self_json["opp_score"] = 0;



    //配置 对手的 json包
    Json::Value opponent_json;
    opponent_json["cmd"] = COMPETEGETQUESTION;
    opponent_json["result"] = result;
    opponent_json["question"] = outJson;
    opponent_json["opp_userName"] = first->getUserName();
    opponent_json["opp_rank"] = rank_map[m_onlineUsers[first->getUserName()]->getRank()];
    opponent_json["opp_score"] = 0;

    //发送回 自己的客户端
   writeData(first, self_json);

   //发送回 对手的客户端
   writeData(second, opponent_json);


}


//处理 答题转发 函数
void MyServer::dealAnswer(TcpSocket * s, const Json::Value &inJson)
{
    //解析 对手 名字
    std::string opp_name = inJson["opp_name"].asString();

    std::unique_lock<std::mutex> answer_lock(online_user_mutex);

    User::ptr& opp = m_onlineUsers[opp_name];


    Json::Value json;
    json["cmd"] = ANSWER;
    json["user_score"] = inJson["user_score"].asInt();  //自己的分数
    json["user_questionId"] = inJson["user_questionId"].asInt();  //自己的答题id

    //将自己的 信息 转发给 对手
    writeData(opp->getSocket(), json);

}


//处理 答题结束后 函数
void MyServer::dealCompeteResult(TcpSocket * s, const Json::Value &inJson)
{

    std::unique_lock<std::mutex> competeResult_lock(compete_mutex);
    User::ptr& user = m_onlineUsers[s->getUserName()];
    
    int user_score = inJson["user_score"].asInt();
    int opp_socre = inJson["opp_score"].asInt();
    std::string opp_name = inJson["opp_name"].asString();

    g_logger->info("player1:{}, score: {},  player2:{}, socre:{}\n",s->getUserName(), opp_name, user_score, opp_socre);

    int rank = user->getRank();

    Json::Value json;
    json["cmd"] = COMPETERESULT;

    if(user_score > opp_socre)
    {
        user->changeRank(rank + 1);
        json["rank_result"] = rank_map[rank + 1];

    }
    else if(user_score < opp_socre)
    {
        user->changeRank(rank - 1);
        json["rank_result"] = rank_map[rank - 1];

        if(rank - 1 <= 0)
            json["rank_result"] = rank_map[0];
    }

     writeData(s, json);

}


}



