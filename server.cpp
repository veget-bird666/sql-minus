// server.cpp
#include "server.h"
#include <QDebug>

Server::Server(QObject *parent) : QTcpServer(parent) {
    if (!listen(QHostAddress::Any, 12345)) {
        qDebug() << "Server could not start!";
    } else {
        qDebug() << "Server started!";
    }
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
    qDebug() << "Received SQL command:" << sqlCommand;

    try {
        Operation* operation = sqlParser.parse(sqlCommand);
        operation->execute();
    } catch (const std::exception& e) {
        clientSocket->write(QString("Error: %1").arg(e.what()).toUtf8());
    }
}

void Server::handleClientDisconnect() {
    qDebug() << "Client disconnected!";
    clientSocket->deleteLater();
}
