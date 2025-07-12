#ifndef CONNECTWINDOW_H
#define CONNECTWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <client.h>
#include <waitingwindow.h>

class ConnectWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectWindow(QWidget *parent = nullptr);
    ~ConnectWindow() = default;

private slots:
    void onConnectClicked();
    void onStatusMessage(const QString& message);
    void onConnectionError(const QString& error);
    void onCountingRequested(int participants, const QString& rhyme);
    void onVerificationReceived(bool isOk);

private:
    void setupUi();

    WaitingWindow* m_waitingWindow;
    Client* m_client;
    QLineEdit* m_nameEdit;
    QLineEdit* m_hostEdit;
    QLineEdit* m_portEdit;
    QPushButton* m_connectButton;
    QLabel* m_statusLabel;
};

#endif // CONNECTWINDOW_H
