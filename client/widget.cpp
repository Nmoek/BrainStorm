#include "widget.h"
#include "ui_widget.h"
#include "register.h"
#include <QMessageBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("头脑风暴登录界面");
    ui->lineEdit_user->setPlaceholderText("请输入用户名");
    ui->lineEdit_passwd->setPlaceholderText("请输入密码");
    ui->lineEdit_passwd->setEchoMode(QLineEdit::Password);


    socket = new ClientSocket(this);


    //由通信类转接过来的 需要处理的数据  登录处理
    connect(socket, &ClientSocket::logResult, this, &Widget::deal_login);
}

Widget::~Widget()
{
    delete ui;
}


// 登录按钮 槽函数
void Widget::on_pushButton_log_clicked()
{
    QString user = ui->lineEdit_user->text();
    QString pass = ui->lineEdit_passwd->text();

    //配置 json包
    QJsonObject json;
    json["cmd"] = LOG;
    json["user"] = user;
    json["passwd"] = pass;

    //向服务器 写入数据
    socket->writeData(json);

}


// 注册 按钮 槽函数
void Widget::on_pushButton_reg_clicked()
{
    this->hide();
    Register reg(socket, this);
    reg.exec(); //模态形式:只有当前对话框可活动，完成当前对话框之后，才能执行其他操作
    this->show();

}


// 处理登录 槽函数
void Widget::deal_login(QJsonObject json)
{
    int result = json["result"].toInt();
    switch (result)
    {
    case OK:
    {
        ui->lineEdit_user->clear();
        ui->lineEdit_passwd->clear();
        this->hide();
        //把接收到并封装好的json数据 传给答题界面
        brain = new BrainStorm(socket, json, this);
        brain->show();

        //发生关闭事件时候 就删除该部件
        brain->setAttribute(Qt::WA_DeleteOnClose);

    }break;

    case ERROR:
    {
        QMessageBox::critical(this, "登录错误", "登录失败,发生未知错误");
    } break;

    case USEREXIST:
    {
        QMessageBox::critical(this, "登录错误", "登录失败,用户已登录");

    }break;

    case NAMEORPASS:
    {
        QMessageBox::critical(this, "登录错误", "登录失败,用户名或密码错误");

    }break;

    case USERLOGREADY:
    {
        QMessageBox::critical(this, "登录错误", "登录失败,用户已经在线");
    } break;


    default:
        break;
    }
}
