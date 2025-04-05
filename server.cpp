// server.cpp
#include "server.h"
#include <QDebug>

Server::Server(QObject *parent) : QTcpServer(parent) {
    if (!listen(QHostAddress::AnyIPv4, 12345)) {
        qDebug() << "Server could not start!";
    } else {
        qDebug() << "Server started!";
    }
}


QString Server::databaselist(){
    QString message;
    try {
        std::vector<DatabaseBlock> databases = FileUtil::readAllDatabaseBlocks();
        for (const auto& db : databases) {
            qDebug() << "数据库名称:" << db.name
                     << "类型:" << (db.type ? "系统" : "用户")
                     << "路径:" << db.filename;

        }
        for (const auto& db : databases) {
            QString dbName = QString::fromUtf8(db.name);
            message+=dbName+'\n';
        }
    } catch (const std::exception& e) {
        qCritical() << "读取数据库列表失败:" << e.what();
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
        clientSocket->write(databaselist().toUtf8());
    }
    else{
    qDebug() << "Received SQL command:" << sqlCommand;

    try {
        Operation* operation = sqlParser.parse(sqlCommand);
        operation->execute();
    } catch (const std::exception& e) {
        clientSocket->write(QString("Error: %1").arg(e.what()).toUtf8());
    }
    }

}

void Server::handleClientDisconnect() {
    qDebug() << "Client disconnected!";
    clientSocket->deleteLater();
}
