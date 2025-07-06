#include "server.h"
#include "clienthandler.h"
#include <QTcpSocket>

Server::Server(QObject* parent) : QTcpServer(parent) {}

void Server::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    ClientHandler* handler = new ClientHandler(socket);
    connect(handler, &ClientHandler::validRequest, this, &Server::validRequest);
    connect(handler, &ClientHandler::invalidRequest, this, &Server::invalidRequest); // Новое соединение
}
