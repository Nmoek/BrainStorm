#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QString>
#include "clientsocket.h"
#include "common.h"
#include "brainstorm.h"


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_pushButton_log_clicked();

    void on_pushButton_reg_clicked();

private:
    Ui::Widget *ui;

    //与服务器通信的 通信类
    ClientSocket *socket;

    //功能主菜单 和 答题界面
    BrainStorm *brain;


    void deal_login(QJsonObject json);
};

#endif // WIDGET_H
