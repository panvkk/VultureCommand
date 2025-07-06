#ifndef CONNECTWINDOW_H
#define CONNECTWINDOW_H

#include <QWidget>
#include <QTcpSocket>
#include <QTimer>

class QLineEdit;
class QPushButton;
class QLabel;

class ConnectWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectWindow(QWidget *parent = nullptr);

private slots:
    void onConnectClicked();
    void onConnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onTimeout();

private:
    void resetConnection();

    QLineEdit *m_surnameEdit;
    QLineEdit *m_ipEdit;
    QLineEdit *m_portEdit;
    QLineEdit *m_messageEdit;
    QPushButton *m_connectButton;
    QLabel *m_statusLabel;
    QTcpSocket *m_socket;
    QTimer *m_timeoutTimer;
};

#endif // CONNECTWINDOW_H
