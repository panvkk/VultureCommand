#include "waitingwindow.h"

WaitingWindow::WaitingWindow(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_statusLabel = new QLabel("Ожидание запроса на считалку от сервера...", this);
    layout->addWidget(m_statusLabel);
    setLayout(layout);
    setWindowTitle("Ожидание");
}

void WaitingWindow::showResultWaiting()
{
    m_statusLabel->setText("Ожидание подтверждения от сервера...");
}
