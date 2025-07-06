#include "client.h"
#include <QDebug>

Client::Client(const QString& studentName, QObject *parent)
    : QObject(parent), m_studentName(studentName), m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &Client::onDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &Client::onError);
}

Client::~Client()
{
    m_socket->deleteLater();
}

void Client::connectToServer(const QString& host, quint16 port)
{
    qDebug() << "Попытка подключения к серверу...";
    m_socket->connectToHost(host, port);
}

void Client::onConnected()
{
    qDebug() << "Успешно подключено к серверу!";
    QString message = QString("Hello, Garson, I'm %1!").arg(m_studentName);
    m_socket->write(message.toUtf8());
    qDebug() << "Отправлено сообщение:" << message;
}

void Client::onReadyRead()
{
    QByteArray response = m_socket->readAll();
    QString responseStr(response);
    qDebug() << "Получен ответ от сервера:" << responseStr;

    if (responseStr.contains("Go To Sleep To the Garden!")) {
        emit showGardenRequested(m_studentName);
    } else {
        qDebug() << "Ошибка в сообщении. Отправляем правильную версию...";
        QString correctMessage = QString("Hello, Garson, I'm %1!").arg(m_studentName);
        m_socket->write(correctMessage.toUtf8());
    }
}

void Client::onDisconnected()
{
    qDebug() << "Отключено от сервера";
}

void Client::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    qDebug() << "Ошибка подключения:" << m_socket->errorString();
}
