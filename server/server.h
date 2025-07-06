#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>

class Server : public QTcpServer {
    Q_OBJECT
public:
    explicit Server(QObject* parent = nullptr);

signals:
    void validRequest(const QString& surname);
    void invalidRequest(const QString& message, const QString& error); // Новый сигнал

protected:
    void incomingConnection(qintptr socketDescriptor) override;
};

#endif // SERVER_H
