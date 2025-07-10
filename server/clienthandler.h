// clienthandler.h
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
    void invalidRequest(const QString& message, const QString& error);

private:
    void processLine(const QByteArray& lineData);
    QString analyzeErrorMessage(const QString& message);

    QTcpSocket* socket;
    QByteArray buffer;
};

#endif // CLIENTHANDLER_H
