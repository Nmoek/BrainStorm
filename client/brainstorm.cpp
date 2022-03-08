#include "brainstorm.h"
#include "ui_brainstorm.h"
#include "widget.h"

#include <QDebug>

BrainStorm::BrainStorm(ClientSocket *socket, QJsonObject json, Widget* wid, QWidget *parent) :
    QDialog(parent)
    ,ui(new Ui::BrainStorm)
    ,m_wid(wid)
{
    ui->setupUi(this);
    this->setWindowTitle("头脑风暴答题界面");
    ui->stackedWidget->setCurrentWidget(ui->mainMenu);

    m_socket = socket;

    //初始化 题目下标为0 即:第一题
    singleQuestion_index = 0;

    //初始化 单人模式的计时器秒数
    single_sec = 10;

    //初始化 单人模式 答题分数
    single_score = 0;

    //匹配模式 自己的名字
    user_name = json["user"].toString();

    // 匹配模式 自己的段位说明
    user_rank = json["rank_explain"].toString();

    user_score = 0;

    //匹配模式 初始化 题目下标为0 即:第一题
    userQuestion_index = 0;

    oppQuestion_index = 0;



    //设置 匹配 模式 等待动画
    ui->label_compete_wait->setStyleSheet("background-color: transparent;");
    m = new QMovie(":/image/loan.gif");
    ui->label_compete_wait->setMovie(m);
    ui->label_compete_wait->setScaledContents(true);
    m->start();



    //个人模式 由通信类转接过来的 需要处理的数据  题目处理
    connect(m_socket, &ClientSocket::singleQuestion, this, &BrainStorm::recv_singleQuestion);

    //个人模式 计时器 槽函数
    connect(&single_timer, &QTimer::timeout, this, &BrainStorm::deal_singleTimeOut);

    // 匹配模式 由通信类转接过来的 需要处理的数据  题目处理
    connect(m_socket, &ClientSocket::competeQuestion, this, &BrainStorm::deal_competeQuestion);

    // 匹配模式 计时器 槽函数
    connect(&compete_timer, &QTimer::timeout, this, &BrainStorm::deal_competeTimeOut);

    //匹配模式 处理 答题转发结果 槽函数
    connect(m_socket, &ClientSocket::answerRsult, this, &BrainStorm::deal_answer);

    //匹配模式 处理 匹配结束结果 槽函数
    connect(m_socket, &ClientSocket::competeResult, this, &BrainStorm::deal_competeResult);


}

BrainStorm::~BrainStorm()
{
    delete ui;
}



//退出登录 按钮
void BrainStorm::on_pushButton_exit_login_clicked()
{
    m_socket->getSocket()->close();
    this->hide();
    m_wid->show();
}


//单机模式 按钮槽函数
void BrainStorm::on_pushButton_signal_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->singleMenu);

}


//单机模式 返回按钮
void BrainStorm::on_pushButton_single_back_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->mainMenu);

}



//单机模式 开始答题按钮 从服务器获取题目
void BrainStorm::on_pushButton_single_begin_clicked()
{
    singleGetQuestion();
}


// 单人模式 向服务器请求题目
void BrainStorm::singleGetQuestion()
{
    // 配置json包
    QJsonObject json;
    json["cmd"] = SINGLEGETQUESTION;

    //发送json包
    m_socket->writeData(json);

}


//单人模式 处理接从服务器收到题目
void BrainStorm::recv_singleQuestion(QJsonObject json)
{
    //从信号中接收带过来的参数
     m_singleQuestion = json;

     // 答题页面设置
     singleSetQuestion();

     //开启定时器
     single_timer.start(1000); //ms单位

    //页面跳转
    ui->stackedWidget->setCurrentWidget(ui->single_runing);\
}


//单人模式 将题目显示在界面上
void BrainStorm::singleSetQuestion()
{
    ui->lcdNumber->display(single_sec);
    ui->label_single_question->setText(m_singleQuestion["question"].toArray().at(singleQuestion_index).toString());
    ui->pushButton_option_A->setText(m_singleQuestion["optionA"].toArray().at(singleQuestion_index).toString());
    ui->pushButton_option_B->setText(m_singleQuestion["optionB"].toArray().at(singleQuestion_index).toString());
    ui->pushButton_option_C->setText(m_singleQuestion["optionC"].toArray().at(singleQuestion_index).toString());
    ui->pushButton_option_D->setText(m_singleQuestion["optionD"].toArray().at(singleQuestion_index).toString());


}


// 单人模式 计时器开始计时 槽函数
void BrainStorm::deal_singleTimeOut()
{
    if(--single_sec == -1)
    {
        ui->label_single_result->setText("未在时间结束前答题,本轮答题结束");
        ui->label_single_score->setText(QString("得分: %1").arg(single_score));
        single_timer.stop();
        ui->stackedWidget->setCurrentWidget(ui->singlescore);

    }
    ui->lcdNumber->display(single_sec);

}


//个人模式 得分情况界面 返回按钮
void BrainStorm::on_pushButton_singlesocre_back_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->mainMenu);

}


//个人模式 答题判断
void BrainStorm::single_answer(QString select)
{
    if(select == m_singleQuestion["answer"].toArray().at(singleQuestion_index).toString())
    {
        single_score += 10;
    }
    else
    {
        ui->label_single_result->setText("回答错误,本轮答题结束");
        ui->label_single_score->setText(QString("得分: %1").arg(single_score));
        single_timer.stop();
        ui->stackedWidget->setCurrentWidget(ui->singlescore);

    }

    single_sec = 10;
    singleQuestion_index++;
    if(singleQuestion_index == m_singleQuestion["question"].toArray().size() - 1)
    {
        ui->label_single_result->setText("全部回答正确,本轮答题结束");
        ui->label_single_score->setText(QString("得分: %1").arg(single_score));
        single_timer.stop();
        ui->stackedWidget->setCurrentWidget(ui->singlescore);

    }
    singleSetQuestion();

}


//选项A 槽函数
void BrainStorm::on_pushButton_option_A_clicked()
{
    single_answer("A");
}


//选项B  槽函数
void BrainStorm::on_pushButton_option_B_clicked()
{
    single_answer("B");
}


//选项C  槽函数
void BrainStorm::on_pushButton_option_C_clicked()
{
    single_answer("C");
}


//选项D  槽函数
void BrainStorm::on_pushButton_option_D_clicked()
{
    single_answer("D");
}

/*---------------------------------------------------------------------*/


// 匹配模式 按钮槽函数
void BrainStorm::on_pushButton_compete_clicked()
{
    QJsonObject json;
    json["cmd"] = COMPETE;
    m_socket->writeData(json);


    ui->stackedWidget->setCurrentWidget(ui->compete_wait);

}


//匹配模式 处理接从服务器收到题目
void BrainStorm::deal_competeQuestion(QJsonObject json)
{

    m_competeQuestion = json["question"].toObject();
    opp_name = json["opp_userName"].toString();
    opp_rank = json["opp_rank"].toString();
    opp_score = json["opp_score"].toInt();


    compete_sec = 10;
    user_score = 0;

    //匹配模式 初始化 题目下标为0 即:第一题
    userQuestion_index = 0;

    oppQuestion_index = 0;


    set_oppInfo();
    set_selfInfo();
    competeSetQuestion();

    ui->stackedWidget->setCurrentWidget(ui->compete_runing);
    compete_timer.start(1000);


}


//匹配模式 计时器开始计时槽函数
void BrainStorm::deal_competeTimeOut()
{
    if(--compete_sec == -1)
    {

        compete_answer("E");
        return;

    }
    ui->lcdNumber_compete->display(compete_sec);

}


// 设置对手信息
void BrainStorm::set_oppInfo()
{
    QString temp = QString("对手: %1  rank:%2   score:%3").arg(opp_name).arg(opp_rank).arg(opp_score);
    ui->label_compete_info->setText(temp);
}


//设置 本身信息
void BrainStorm::set_selfInfo()
{
    QString temp = QString("自己: %1  rank:%2   score:%3").arg(user_name).arg(user_rank).arg(user_score);
    ui->label_self_info->setText(temp);

}


//匹配模式 将题目显示在界面上
void BrainStorm::competeSetQuestion()
{

    ui->lcdNumber_compete->display(compete_sec);

    // 清空 按钮设置
    ui->pushButton_option_A_2->setEnabled(true);
    ui->pushButton_option_B_2->setEnabled(true);
    ui->pushButton_option_C_2->setEnabled(true);
    ui->pushButton_option_D_2->setEnabled(true);
    ui->pushButton_option_A_2->setStyleSheet("");
    ui->pushButton_option_B_2->setStyleSheet("");
    ui->pushButton_option_C_2->setStyleSheet("");
    ui->pushButton_option_D_2->setStyleSheet("");

    if(oppQuestion_index == m_competeQuestion["question"].toArray().size())
    {

        compete_timer.stop();

        // 配置对战 结束json包
        QJsonObject json;
        json["cmd"] = COMPETERESULT;
        json["opp_name"] = opp_name;
        json["user_score"] = user_score;
        json["opp_score"] = opp_score;

        //发送给 服务器
        m_socket->writeData(json);

        ui->stackedWidget->setCurrentWidget(ui->compete_score);

        return;


    }

    if(userQuestion_index < m_competeQuestion["question"].toArray().size())
    {

        ui->label_compete_question->setText(m_competeQuestion["question"].toArray().at(userQuestion_index).toString());
        ui->pushButton_option_A_2->setText(m_competeQuestion["optionA"].toArray().at(userQuestion_index).toString());
        ui->pushButton_option_B_2->setText(m_competeQuestion["optionB"].toArray().at(userQuestion_index).toString());
        ui->pushButton_option_C_2->setText(m_competeQuestion["optionC"].toArray().at(userQuestion_index).toString());
        ui->pushButton_option_D_2->setText(m_competeQuestion["optionD"].toArray().at(userQuestion_index).toString());
    }
}


//匹配模式 答题判断
void BrainStorm::compete_answer(QString select)
{
    compete_timer.stop();
    if(select == m_competeQuestion["answer"].toArray().at(userQuestion_index).toString())
    {
        user_score += 10;
    }

    set_selfInfo();
    userQuestion_index++;




    //自己和对手题目一样 才跳到下一题
    if(userQuestion_index == oppQuestion_index)
    {

            compete_timer.stop();
            compete_sec = 10;

            competeSetQuestion();

            compete_timer.start(1000);
    }



    // 告诉对手 自己的相关信息
    QJsonObject user_json;
    user_json["cmd"] = ANSWER;
    user_json["opp_name"] = opp_name;
    user_json["user_score"] = user_score;
    user_json["user_questionId"] = userQuestion_index;

    m_socket->writeData(user_json);



}


//匹配模式 选项A
void BrainStorm::on_pushButton_option_A_2_clicked()
{
    ui->pushButton_option_A_2->setStyleSheet("background-color: rgb(0, 255, 0)");
    ui->pushButton_option_A_2->setEnabled(false);
    ui->pushButton_option_B_2->setEnabled(false);
    ui->pushButton_option_C_2->setEnabled(false);
    ui->pushButton_option_D_2->setEnabled(false);
    compete_answer("A");

}


//匹配模式 选项B
void BrainStorm::on_pushButton_option_B_2_clicked()
{
    ui->pushButton_option_B_2->setStyleSheet("background-color: rgb(0, 255, 0)");
    ui->pushButton_option_A_2->setEnabled(false);
    ui->pushButton_option_B_2->setEnabled(false);
    ui->pushButton_option_C_2->setEnabled(false);
    ui->pushButton_option_D_2->setEnabled(false);

    compete_answer("B");
}


//匹配模式 选项C
void BrainStorm::on_pushButton_option_C_2_clicked()
{
    ui->pushButton_option_C_2->setStyleSheet("background-color: rgb(0, 255, 0)");
    ui->pushButton_option_A_2->setEnabled(false);
    ui->pushButton_option_B_2->setEnabled(false);
    ui->pushButton_option_C_2->setEnabled(false);
    ui->pushButton_option_D_2->setEnabled(false);
    compete_answer("C");

}


//匹配模式 选项D
void BrainStorm::on_pushButton_option_D_2_clicked()
{
    ui->pushButton_option_D_2->setStyleSheet("background-color: rgb(0, 255, 0)");
    ui->pushButton_option_A_2->setEnabled(false);
    ui->pushButton_option_B_2->setEnabled(false);
    ui->pushButton_option_C_2->setEnabled(false);
    ui->pushButton_option_D_2->setEnabled(false);
    compete_answer("D");

}


// 匹配模式 答题转发结果 槽函数
void BrainStorm::deal_answer(QJsonObject json)
{

    //接收到对手的 分数和当前题目下标
    opp_score = json["user_score"].toInt();
    oppQuestion_index = json["user_questionId"].toInt();


    set_oppInfo();

    if(userQuestion_index == oppQuestion_index)
    {

            compete_timer.stop();
            compete_sec = 10;

            competeSetQuestion();

           compete_timer.start(1000);
    }

}


//匹配模式 处理 答题结束结果 槽函数
void BrainStorm::deal_competeResult(QJsonObject json)
{
    QString rank_result = json["rank_result"].toString();

    if(user_score > opp_score)
    {
        ui->label_compete_result->setText(QString("胜利! 你的得分:%1  对手%2得分: %3").arg(user_score).arg(opp_name).arg(opp_score));
    }
    else if(user_score < opp_score)
    {
        ui->label_compete_result->setText(QString("失败! 你的得分:%1  对手%2得分: %3").arg(user_score).arg(opp_name).arg(opp_score));

    }
    else if(user_score == opp_score)
    {
        ui->label_compete_result->setText(QString("平局! 你的得分:%1  对手%2得分: %3").arg(user_score).arg(opp_name).arg(opp_score));
        rank_result = user_rank;
    }




    QString temp = QString("%1 ---------> %2").arg(user_rank).arg(rank_result);

    ui->label_rank_result->setText(temp);



}


//匹配模式 答题结果 返回按钮槽函数
void BrainStorm::on_pushButton_compete_back_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->mainMenu);
}



