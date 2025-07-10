// clienthandler.cpp
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

    QRegularExpression regex("^Hello, Garson, I'm ([A-Za-z]+)!$");
    QRegularExpressionMatch match = regex.match(message);

    if(match.hasMatch()) {
        QString surname = match.captured(1);
        QString response = "I'm not Garson, I'm Server! Go To Sleep To the Garden!\n";
        emit validRequest(surname);

        socket->write(response.toUtf8());
        socket->flush();
    } else {
        QString error = analyzeErrorMessage(message);
        QString response = "ERROR: " + error + "\n";

        emit invalidRequest(message, error);
        socket->write(response.toUtf8());
        socket->flush();
    }
}

QString ClientHandler::analyzeErrorMessage(const QString& message) {
    QStringList errors;

    if (!message.startsWith("Hello, ")) {
        errors << "Message must start with 'Hello, '";
    }

    if (!message.contains("Garson,")) {
        errors << "Must contain 'Garson,' after 'Hello, '";
    } else if (message.indexOf("Garson,") != 7) {
        errors << "Missing space after 'Hello,'";
    }

    if (!message.contains("I'm ")) {
        errors << "Must contain 'I'm ' before surname";
    }

    if (!message.endsWith("!")) {
        errors << "Message must end with '!'";
    }

    QRegularExpression nameRegex("I'm ([A-Za-z]+)!");
    if (!nameRegex.match(message).hasMatch()) {
        errors << "Surname must contain only letters after 'I'm '";
    }

    if (errors.isEmpty()) {
        errors << "Unknown message format error";
    }

    return errors.join(". ") + ". Correct format: 'Hello, Garson, I'm [Surname]!'";
}
