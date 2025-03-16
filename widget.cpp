#include "widget.h"
#include "./ui_widget.h"
#include "operation.h"
#include "sqlparser.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
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

    QString sql_command = ui->commandEdit->toPlainText();
    try {
        Operation* o = SqlParser::parse(sql_command);
        o->execute();
    } catch (...) {
        qDebug()<<"command is not correct";
    }


}

