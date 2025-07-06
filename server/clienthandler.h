#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QRegularExpression>

class ClientHandler : public QObject {
    Q_OBJECT
public:
    explicit ClientHandler(QTcpSocket* socket, QObject* parent = nullptr);

public slots:
    void readData();

signals:
    void validRequest(const QString& surname);
    void invalidRequest(const QString& message, const QString& error); // Новый сигнал

private:
    void processLine(const QByteArray& lineData);

    QTcpSocket* socket;
    QByteArray buffer;
};

#endif // CLIENTHANDLER_H
