// server.h
#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include "sqlparser.h"
#include "database_manager.h"
#include "file_utils.h"

class Server : public QTcpServer {
    Q_OBJECT
public:
    Server(QObject *parent = nullptr);

    QString databaselist();

    void incomingConnection(qintptr socketDescriptor) override;

    void readClientData();
    void handleClientDisconnect();


    QTcpSocket *clientSocket;
    SqlParser sqlParser;
};

#endif // SERVER_H
