#include "clienthandler.h"
#include <QByteArray>
#include <QRegularExpression>

ClientHandler::ClientHandler(QTcpSocket* socket, QObject* parent)
    : QObject(parent), socket(socket) {
    connect(socket, &QTcpSocket::readyRead, this, &ClientHandler::readData);
    connect(socket, &QTcpSocket::disconnected, this, &ClientHandler::deleteLater);
}

void ClientHandler::readData() {
    buffer.append(socket->readAll());

    int endIndex;
    while ((endIndex = buffer.indexOf('\n')) != -1) {
        QByteArray lineData = buffer.left(endIndex);
        buffer = buffer.mid(endIndex + 1);
        processLine(lineData);
    }
}

void ClientHandler::processLine(const QByteArray& lineData) {
    QString message = QString::fromUtf8(lineData).trimmed();

    QRegularExpression regex("Hello, Garson, I'm ([A-Za-z]+)!");
    QRegularExpressionMatch match = regex.match(message);

    QString response;
    if(match.hasMatch()) {
        QString surname = match.captured(1);
        response = "I'm not Garson, I'm Server! Go To Sleep To the Garden!\n";
        emit validRequest(surname);

        socket->write(response.toUtf8());
        socket->flush();
        socket->disconnectFromHost();
    } else {
        QString error = "Invalid message format";
        response = "ERROR: " + error + "! Use: 'Hello, Garson, I'm [Surname]!'\n";

        // Отправляем сигнал об ошибке
        emit invalidRequest(message, error);

        socket->write(response.toUtf8());
        socket->flush();
    }
}
