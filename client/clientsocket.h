#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include "common.h"
#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument> //json数据解析
#include <QHostAddress>

class ClientSocket : public QObject
{
    Q_OBJECT
public:
    explicit ClientSocket(QObject *parent = 0);

    //向服务器 写入数据
    void writeData(const QJsonObject &json);

    /**
     * @brief 获取tcp套接字对象
     * @return
     */
    QTcpSocket* getSocket() const {return socket;}

signals:
    // 注册结果 信号
    void regResult(int);

    //登录结果信号
    void logResult(QJsonObject json);

    // 单机题目获取结果信号
    void singleQuestion(QJsonObject json);

    //匹配题目获取信号
    void competeQuestion(QJsonObject json);

    //匹配模式 答题转发的结果
    void answerRsult(QJsonObject json);

     //匹配模式 答题结束的结果
    void competeResult(QJsonObject json);

public slots:
    // 读取服务器 数据
    void readData();

private:
    QTcpSocket *socket;

};

#endif // CLIENTSOCKET_H
