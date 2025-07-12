#include "client.h"
#include <QDataStream>
#include <QMessageBox>
#include <QFile>
#include <mainwindow.h>

Client::Client(QObject *parent) : QObject(parent),
    m_socket(new QTcpSocket(this)),
    m_timeoutTimer(new QTimer(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &Client::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &Client::onError);
    connect(m_timeoutTimer, &QTimer::timeout, this, &Client::onTimeout);
}

void Client::connectToServer(const QString& host, quint16 port, const QString& clientName)
{
    m_clientName = clientName;
    resetConnection();
    m_socket->connectToHost(host, port);
    m_timeoutTimer->start(10000);
}

void Client::resetConnection()
{
    if (m_socket) {
        m_socket->abort(); // Прерываем текущее соединение
    }
    if (m_timeoutTimer) {
        m_timeoutTimer->stop(); // Останавливаем таймер
    }
}

void Client::processServerRequest(const QByteArray& data) {
    QString message = QString::fromUtf8(data).trimmed();

    if (message.startsWith("COUNTING_TASK:")) {
        QStringList parts = message.split(':');
        if (parts.size() >= 3) {
            int participants = parts[1].toInt();
            QString rhyme = parts.mid(2).join(':');
            emit countingRequested(participants, rhyme);
        }
    }
    else if (message == "VERIFICATION_OK") {
        emit verificationReceived(true);
    }
    else if (message == "VERIFICATION_FAILED") {
        emit verificationReceived(false);
    }
    else {
        emit statusMessage("Получено: " + message);
    }
}

void Client::onReadyRead()
{
    m_timeoutTimer->stop(); // Сброс таймаута при получении данных

    QByteArray data = m_socket->readAll();
    QString message = QString::fromUtf8(data).trimmed();

    if (message.startsWith("COUNTING_TASK:")) {
        QStringList parts = message.split(':');
        if (parts.size() >= 3) {
            int participants = parts[1].toInt();
            QString rhyme = parts.mid(2).join(':');
            emit countingRequested(participants, rhyme);
        }
    }
    else if (message == "VERIFICATION_OK") {
        emit verificationReceived(true);
    }
    else if (message == "VERIFICATION_FAILED") {
        emit verificationReceived(false);
    }
    else {
        emit statusMessage("Получено: " + message);
    }
}

// Обработка таймаута соединения
void Client::onTimeout()
{
    m_socket->abort();
    emit connectionError("Превышено время ожидания подключения");
}

// Обработка ошибок сокета
void Client::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    m_timeoutTimer->stop();
    emit connectionError(m_socket->errorString());
}

// Обработка отключения от сервера
void Client::onDisconnected()
{
    m_timeoutTimer->stop();
    emit statusMessage("Отключено от сервера");
}

// Деструктор Client
Client::~Client()
{
    m_socket->disconnectFromHost();
    delete m_socket;
    delete m_timeoutTimer;
}

// Остальные методы Client
void Client::onConnected()
{
    m_timeoutTimer->stop();
    emit statusMessage("Подключено. Ожидание задания...");
    m_socket->write(QString("CLIENT_READY:%1").arg(m_clientName).toUtf8());
}

void Client::sendCountingResult(const WinnerReport& report)
{
    if (m_socket->state() == QTcpSocket::ConnectedState) {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_6_0);

        out << quint8(report.day)
            << quint8(report.month)
            << quint16(report.year)
            << quint8(report.startingParticipant)
            << quint8(report.winnerNumber)
            << QByteArray(report.winnerName, 20)
            << quint8(report.hours)
            << quint8(report.minutes)
            << quint8(report.seconds);

        m_socket->write(block);
    }
}

void Client::saveStatistics()
{
    QFile file("statistics.dat");
    if (file.open(QIODevice::Append)) {
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_6_0);
        out << m_lastReport.day << m_lastReport.month << m_lastReport.year
            << m_lastReport.startingParticipant << m_lastReport.winnerNumber
            << QByteArray(m_lastReport.winnerName, 20) << m_lastReport.hours
            << m_lastReport.minutes << m_lastReport.seconds;
        file.close();
    }
}
