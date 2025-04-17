#include "widget.h"
#include <QApplication>
#include <QString>
Widget* widget;
QString currentDB = "";


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("sql-minus");
    widget = &w;
    w.show();
    return a.exec();
}


/*目前可解析语句示例：
 *
 * show databases;
 * create database db01;
 * drop database db02;
 * use database db03;
 * show tables;
 * create table t04(...);
 * drop table t04;
 * alter table t05 add column c01 int not null;
 * alter table t06 drop column c02;
 * alter table t07 modify column c03 int;
 *
 */
