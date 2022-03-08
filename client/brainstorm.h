#ifndef BRAINSTORM_H
#define BRAINSTORM_H

#include <QDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QMovie>

#include "clientsocket.h"

class Widget;


namespace Ui {
class BrainStorm;
}

class BrainStorm : public QDialog
{
    Q_OBJECT

public:
    explicit BrainStorm(ClientSocket *socket, QJsonObject json, Widget *wid, QWidget *parent = 0);
    ~BrainStorm();


private slots:

    //单人模式 从服务器接收到题目 处理题目 槽函数
    void  recv_singleQuestion(QJsonObject json);

    //单人模式 计时器开始计时 槽函数
    void deal_singleTimeOut();

    //单人模式 计时器开始计时 槽函数
    void deal_competeTimeOut();

    //匹配模式  从服务器接收到题目 处理题目 槽函数
    void deal_competeQuestion(QJsonObject json);

    //匹配模式 处理 答题转发结果 槽函数
    void deal_answer(QJsonObject json);

    //匹配模式 处理 答题结束结果 槽函数
    void deal_competeResult(QJsonObject json);

    void on_pushButton_signal_clicked();

    void on_pushButton_single_back_clicked();

    void on_pushButton_single_begin_clicked();

    void on_pushButton_singlesocre_back_clicked();

    void on_pushButton_option_A_clicked();


    void on_pushButton_option_B_clicked();

    void on_pushButton_option_C_clicked();

    void on_pushButton_option_D_clicked();

    void on_pushButton_compete_clicked();

    void on_pushButton_option_A_2_clicked();

    void on_pushButton_option_B_2_clicked();

    void on_pushButton_option_C_2_clicked();

    void on_pushButton_option_D_2_clicked();

    void on_pushButton_compete_back_clicked();

public slots:
    void on_pushButton_exit_login_clicked();

private:
    Ui::BrainStorm *ui;
    Widget* m_wid;
    /*-------------------单人模式------------------*/

    // 与服务器 通信的套接字
    ClientSocket *m_socket;

    // 保存单人模式 题目的json对象
    QJsonObject m_singleQuestion;

    //记录 当前题目的 下标
    int singleQuestion_index;

    //单人模式 计时器
    QTimer single_timer;

    //单人模式 当前计时秒数
    int single_sec;

    //单人模式 得分
    int single_score;


    //向服务器 请求发送题目
    void singleGetQuestion();

    //单人模式 题目设置
    void singleSetQuestion();

    //个人模式 答题判断
    void single_answer(QString select);



    /*---------------------匹配模式---------------------*/

    //匹配模式 动画对象
    QMovie *m;

    //匹配模式 自己的名字
    QString user_name;

    //匹配模式 自己的段位说明
    QString user_rank;

    //匹配模式 自己的得分
    int user_score;

    // 匹配模式 的题目
    QJsonObject m_competeQuestion;

    //匹配模式 对手的名字
    QString opp_name;

    //匹配模式 对手的得分
    int opp_score;

    //匹配模式 对手的rank分
    QString opp_rank;

    //匹配模式 答题计时秒数
    int compete_sec;

    //匹配模式 计时器
    QTimer compete_timer;

    //记录 自己 当前题目的 下标
    int userQuestion_index;

    //记录 对手 当前题目的 下标
    int oppQuestion_index;




    //设置 对手的信息
    void set_oppInfo();

    // 设置 自己的信息
    void set_selfInfo();

    //设置 题目
    void competeSetQuestion();

    //个人模式 答题判断
    void compete_answer(QString select);






};

#endif // BRAINSTORM_H
