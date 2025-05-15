// // 亲爱的候同学，为了便于我们小组高效地测试程序，现已将您的代码注释掉并放在结尾；相应的，我们把一个老版本的代码放在了开头。
// // 如果需要，您可以注释掉或删除掉老代码，然后利用“ctrl + /”回复您的代码，继续对其进行修改
// #include "widget.h"
// #include "./ui_widget.h"
// #include "operation.h"
// #include "sqlparser.h"
// #include "server.h"
// #include <QFile>
// #include <QFileDialog>
// #include <QDir>
// #include <QDebug>

// extern QString currentDB;
// using namespace std;

// Widget::Widget(QWidget *parent)
//     : QWidget(parent)
//     , ui(new Ui::Widget)
// {
//     ui->setupUi(this);

//     server=new Server(this);

// }

// Widget::~Widget()
// {
//     delete ui;
// }



// // 清除按键
// void Widget::on_clearButton_clicked()
// {
//     ui->commandEdit->setText("");
// }


// void Widget::on_executeButton_clicked()
// {

//     QString sqlCommands = ui->commandEdit->toPlainText();

//     try {
//         int successCount = 0;
//         int failCount = 0;

//         SqlParser::executeMulti(sqlCommands, [&](const QString& sql) {
//             try {
//                 executeSqlStatement(sql);
//                 successCount++;
//             } catch (const std::exception& e) {
//                 showMessage(QString("执行失败: ") + e.what());
//                 failCount++;
//             }
//         });

//         showMessage(QString("执行完成: 成功 %1 条, 失败 %2 条").arg(successCount).arg(failCount));
//     } catch (const std::exception& e) {
//         showMessage(QString("解析SQL失败: ") + e.what());
//     }

//     // QString sql_command = ui->commandEdit->toPlainText();
//     // //QString sql_command = server.
//     // try {

//         //     Operation* o = SqlParser::parse(sql_command);
//         //     o->execute();
//         // } catch (const std::exception& e) {
//         //     showMessage(QString("错误: ") + e.what());
//         // } catch (...){
//         //     showMessage("SQL command is not correct.");
//         // }
// }


// // 粘贴文本操作
// void Widget::showMessage(QString message){
//     ui->informationEdit->append(message);

//     // server->clientSocket->write(message.toUtf8());
//     // server->clientSocket->flush();


// }

// // 解析sql脚本功能
// void Widget::on_analysisButton_clicked()
// {
//     QString filePath = QFileDialog::getOpenFileName(this,
//                                                     tr("选择SQL脚本文件"),
//                                                     QDir::currentPath(),
//                                                     tr("SQL文件 (*.sql);;所有文件 (*.*)"));

//     if (filePath.isEmpty()) {
//         return;
//     }

//     try {
//         int successCount = 0;
//         int failCount = 0;

//         SqlParser::executeFromFile(filePath, [&](const QString& sql) {
//             try {
//                 executeSqlStatement(sql);
//                 successCount++;
//             } catch (const std::exception& e) {
//                 showMessage(QString("执行失败: ") + e.what());
//                 failCount++;
//             }
//         });

//         showMessage(QString("执行完成: 成功 %1 条, 失败 %2 条").arg(successCount).arg(failCount));
//     } catch (const std::exception& e) {
//         showMessage(QString("解析脚本失败: ") + e.what());
//     }
// }



// // 执行单挑语句
// void Widget::executeSqlStatement(const QString& sql)
// {
//     // 检查是否以分号结尾，如果没有则添加
//     QString adjustedSql = sql.trimmed();
//     if (!adjustedSql.endsWith(';')) {
//         adjustedSql += ";";
//     }

//     qDebug()<<adjustedSql;
//     Operation* op = SqlParser::parse(adjustedSql);
//     if (op) {
//         op->execute();
//         delete op;
//     }
// }











#include "widget.h"
#include "./ui_widget.h"
#include "operation.h"
#include "sqlparser.h"
#include "server.h"
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QDebug>

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

    QString sqlCommands = ui->commandEdit->toPlainText();

    try {
        int successCount = 0;
        int failCount = 0;

        SqlParser::executeMulti(sqlCommands, [&](const QString& sql) {
            try {
                executeSqlStatement(sql);
                successCount++;
            } catch (const std::exception& e) {
                showMessage(QString("执行失败: ") + e.what());
                failCount++;
            }
        });

        showMessage(QString("执行完成: 成功 %1 条, 失败 %2 条").arg(successCount).arg(failCount));
    } catch (const std::exception& e) {
        showMessage(QString("解析SQL失败: ") + e.what());
    }

    // QString sql_command = ui->commandEdit->toPlainText();
    // //QString sql_command = server.
    // try {

    //     Operation* o = SqlParser::parse(sql_command);
    //     o->execute();
    // } catch (const std::exception& e) {
    //     showMessage(QString("错误: ") + e.what());
    // } catch (...){
    //     showMessage("SQL command is not correct.");
    // }
}


// 粘贴文本操作
void Widget::showMessage(QString message){
    ui->informationEdit->append(message);

    server->clientSocket->write(message.toUtf8());
    server->clientSocket->flush();


}

// 解析sql脚本功能
void Widget::on_analysisButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("选择SQL脚本文件"),
                                                    QDir::currentPath(),
                                                    tr("SQL文件 (*.sql);;所有文件 (*.*)"));

    if (filePath.isEmpty()) {
        return;
    }

    try {
        int successCount = 0;
        int failCount = 0;

        SqlParser::executeFromFile(filePath, [&](const QString& sql) {
            try {
                executeSqlStatement(sql);
                successCount++;
            } catch (const std::exception& e) {
                showMessage(QString("执行失败: ") + e.what());
                failCount++;
            }
        });

        showMessage(QString("执行完成: 成功 %1 条, 失败 %2 条").arg(successCount).arg(failCount));
    } catch (const std::exception& e) {
        showMessage(QString("解析脚本失败: ") + e.what());
    }
}



// 执行单挑语句
void Widget::executeSqlStatement(const QString& sql)
{
    // 检查是否以分号结尾，如果没有则添加
    QString adjustedSql = sql.trimmed();
    if (!adjustedSql.endsWith(';')) {
        adjustedSql += ";";
    }

    qDebug()<<adjustedSql;
    Operation* op = SqlParser::parse(adjustedSql);
    if (op) {
        op->execute();
        delete op;
    }
}
