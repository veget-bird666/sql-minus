// server.cpp
#include "server.h"
#include <QDebug>
#include "widget.h"
extern Widget* widget;

Server::Server(QObject *parent) : QTcpServer(parent) {
    if (!listen(QHostAddress::AnyIPv4, 12345)) {
        qDebug() << "Server could not start!";
    } else {
        qDebug() << "Server started!";
    }
}


QString Server::databaseAndTableList() {
    QString message;
    try {
        // 1. 先读取所有数据库
        auto dbs = FileUtil::readAllDatabaseBlocks();
        for (const auto& db : dbs) {
            QString dbName = QString::fromStdString(db.name);  // 将 std::string 转换为 QString
            message += dbName + "\n";

            // 2. 读取该数据库下的所有表
            QString dbNameQString = QString::fromStdString(db.name);  // 先转换为 QString
            auto tables = FileUtil::readAllTableBlocks(dbNameQString);  // 传递 QString 而不是 std::string
            for (const auto& tb : tables) {
                QString tbName = QString::fromStdString(tb.name);  // 将 std::string 转换为 QString
                message += "\t" + tbName + "\n";
            }
        }
        message += "\n\n"; // 双换行表示结束
    } catch (const std::exception& e) {
        message += QString("Error: %1\n\n").arg(e.what());
    }
    return message;
}


void Server::incomingConnection(qintptr socketDescriptor) {
    clientSocket = new QTcpSocket(this);
    clientSocket->setSocketDescriptor(socketDescriptor);

    connect(clientSocket, &QTcpSocket::readyRead, this, &Server::readClientData);
    connect(clientSocket, &QTcpSocket::disconnected, this, &Server::handleClientDisconnect);

    qDebug() << "Client connected!";
}

void Server::readClientData() {
    QByteArray sqlComman = clientSocket->readAll();
    QString sqlCommand=QString::fromUtf8(sqlComman);
    if(sqlCommand=="1"){
        clientSocket->write(databaseAndTableList().toUtf8());
    }
    else{
    qDebug() << "Received SQL command:" << sqlCommand;

    try {
        //Operation* operation = sqlParser.parse(sqlCommand);
        //operation->execute();
        int successCount = 0;
        int failCount = 0;

        SqlParser::executeMulti(sqlCommand, [&](const QString& sql) {
            try {
                widget->executeSqlStatement(sql);
                successCount++;
            } catch (const std::exception& e) {
                widget->showMessage(QString("执行失败: ") + e.what());
                failCount++;
            }
        });

        widget->showMessage(QString("执行完成: 成功 %1 条, 失败 %2 条").arg(successCount).arg(failCount));
    } catch (const std::exception& e) {
        widget->showMessage(QString("解析SQL失败: ") + e.what());
    }
    }

}

void Server::handleClientDisconnect() {
    qDebug() << "Client disconnected!";
    clientSocket->deleteLater();
}

