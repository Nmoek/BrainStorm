#include "register.h"
#include "ui_register.h"
#include <QMessageBox>
#include <QDebug>

Register::Register(ClientSocket *socket, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
    this->setWindowTitle("头脑风暴注册界面");

    m_socket = socket;

    ui->lineEdit_user->setPlaceholderText("请输入用户名");
    ui->lineEdit_passwd->setPlaceholderText("请输入密码");
    ui->lineEdit_passwd2->setPlaceholderText("请再次确认密码");
    ui->lineEdit_passwd->setEchoMode(QLineEdit::Password);
    ui->lineEdit_passwd2->setEchoMode(QLineEdit::Password);

    //由通信类转接过来的 需要处理的数据  注册处理
    connect(socket, &ClientSocket::regResult, this, &Register::deal_regResult);
}

Register::~Register()
{
    delete ui;
}

// 立即注册按钮 槽函数
void Register::on_pushButton_reg_clicked()
{
    QString user = ui->lineEdit_user->text();
    QString passwd = ui->lineEdit_passwd->text();
    QString passwd2 = ui->lineEdit_passwd2->text();


    if(passwd != passwd2)
    {
        QMessageBox::information(this, "密码问题", "两次输入的密码不一致,重新输入");
        ui->lineEdit_passwd->clear();
        ui->lineEdit_passwd2->clear();
        return;

    }


    //配置注册信息
    QJsonObject json;
    json["cmd"] = REG;
    json["user"] = user;
    json["passwd"] = passwd;

    //向 服务器发送json数据
    m_socket->writeData(json);

}


// 返回按钮 槽函数
void Register::on_pushButton_back_clicked()
{
    this->close();
}


// 处理 注册结果 的槽函数
void Register::deal_regResult(int ret)
{
    switch (ret)
    {
    case OK:
    {
        QMessageBox::information(this, "注册提示","注册已经成功!");
        this->close();
    }break;

    case ERROR:
    {
        QMessageBox::critical(this, "注册错误", "注册错误,发生未知错误");
        ui->lineEdit_user->clear();
        ui->lineEdit_passwd->clear();
        ui->lineEdit_passwd2->clear();
        this->close();
    }break;

    case USEREXIST:
    {
        QMessageBox::critical(this, "注册错误", "注册错误,用户已经存在");
        ui->lineEdit_user->clear();
        ui->lineEdit_passwd->clear();
        ui->lineEdit_passwd2->clear();
    }break;

    default: break;
    }

}
