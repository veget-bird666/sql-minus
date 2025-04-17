#include "widget.h"
#include "./ui_widget.h"
#include "operation.h"
#include "sqlparser.h"
#include "server.h"
extern QString currentDB;
using namespace std;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    server=new Server(this);

}

Widget::~Widget()
{
    delete ui;
}



// 清除按键
void Widget::on_clearButton_clicked()
{
    ui->commandEdit->setText("");
}


void Widget::on_executeButton_clicked()
{

    //QString sql_command = server.
    /*try {

        Operation* o = SqlParser::parse(sql_command);
        o->execute();
    } catch (...) {
        showMessage("SQL command is not correct.");
    }
}

// 粘贴文本操作
void Widget::showMessage(QString message){
    ui->informationEdit->append(message);
    server->clientSocket->write(message.toUtf8());
    server->clientSocket->flush();

}
