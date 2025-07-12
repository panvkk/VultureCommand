#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QObject>
#include <QString>
#include <QTimer>
#include "winnerreport.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    ~Client();
    void resetConnection();
    void saveStatistics();
    void connectToServer(const QString& host, quint16 port, const QString& clientName);
    void sendCountingResult(const WinnerReport& report);

signals:
    void countingRequested(int participants, const QString& rhyme);
    void verificationReceived(bool isOk);
    void connectionError(const QString& error);
    void statusMessage(const QString& message);

private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onTimeout();

private:
    void processServerRequest(const QByteArray& data);

    QTcpSocket *m_socket;
    QTimer *m_timeoutTimer;
    QString m_clientName;
    WinnerReport m_lastReport; // Добавлено для хранения последнего отчета
};

#endif // CLIENT_H
