#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include "common.h"
#include "clientsocket.h"


namespace Ui {
class Register;
}

class Register : public QDialog
{
    Q_OBJECT

public:
    explicit Register(ClientSocket *socket, QWidget *parent = 0);
    ~Register();

private slots:
    void on_pushButton_reg_clicked();

    void on_pushButton_back_clicked();

    void deal_regResult(int);

private:
    Ui::Register *ui;

    ClientSocket *m_socket;


};

#endif // REGISTER_H
