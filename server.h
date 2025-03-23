// server.h
#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include "sqlparser.h"
#include "database_manager.h"

class Server : public QTcpServer {
    Q_OBJECT
public:
    Server(QObject *parent = nullptr);

    void incomingConnection(qintptr socketDescriptor) override;

    void readClientData();
    void handleClientDisconnect();

    QTcpSocket *clientSocket;
    SqlParser sqlParser;
};

#endif // SERVER_H
