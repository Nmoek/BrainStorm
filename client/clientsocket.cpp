#include "clientsocket.h"

ClientSocket::ClientSocket(QObject *parent) : QObject(parent)
{
    socket = new QTcpSocket(this);

    socket->connectToHost(QHostAddress("192.168.77.136"), 9999);


    //每当 收到 服务器的数据时 触发readyRead信号
    connect(socket, &QTcpSocket::readyRead, this, &ClientSocket::readData);

}



// 接收 服务器的数据 的槽函数
void ClientSocket::readData()
{
    QByteArray data;
    //检查数据 是否已经读完
    while(socket->bytesAvailable())
    {
        data += socket->readAll();

    }

    //数据解析
    QJsonDocument js_data = QJsonDocument::fromJson(data);
    if(js_data.isNull())
        return;


    QJsonObject root = js_data.object(); //json对象

    //具体逻辑处理: 以发送信号的方式 转接到各个界面中 去处理相关事宜
    int cmd = root["cmd"].toInt();
    switch(cmd)
    {
    case REG:
    {
        //接收到服务器 对注册响应 的结果 将其发送给注册界面类
        emit regResult(root["result"].toInt());
    }
        break;
    case LOG:
    {
        //接收到服务器 对登录响应 的结果 将其发送给登录界面类
        emit logResult(root);
    }
        break;
    case SINGLEGETQUESTION:
    {
        //接收到服务器 对请求题目响应 的结果 将其发送给答题界面类
        emit singleQuestion(root["question"].toObject());
    }
        break;

    case COMPETEGETQUESTION:
    {
        //接收到服务器 对请求题目响应 的结果 将其发送给答题界面类
        emit competeQuestion(root);

    }
        break;

    case ANSWER:
    {
        emit answerRsult(root);
    }
        break;

    case COMPETERESULT:
    {
        emit competeResult(root);

    }
        break;

    default:
        break;
    }

}


// 向服务器 写数据
void ClientSocket::writeData(const QJsonObject &json)
{
    QJsonDocument data(json);
    QByteArray send_data = data.toJson(); //转为字节数组形式 测量大小
    int send_len = send_data.size();

    //先发 数据长度 再发数据本身  write库函数
    socket->write((char*)&send_len, sizeof(int));
    socket->write(send_data);




}

