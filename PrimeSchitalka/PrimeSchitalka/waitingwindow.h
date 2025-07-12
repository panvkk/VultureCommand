#ifndef WAITINGWINDOW_H
#define WAITINGWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class WaitingWindow : public QWidget
{
    Q_OBJECT
public:
    explicit WaitingWindow(QWidget *parent = nullptr);
    void showResultWaiting();

private:
    QLabel *m_statusLabel;
};

#endif // WAITINGWINDOW_H
