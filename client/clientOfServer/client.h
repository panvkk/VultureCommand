#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QObject>
#include <QString>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(const QString& studentName, QObject *parent = nullptr);
    ~Client();

    void connectToServer(const QString& host, quint16 port);

signals:
    void showGardenRequested(const QString& studentName);

private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    QString m_studentName;
    QTcpSocket *m_socket;
};

#endif // CLIENT_H
