// connectwindow.cpp
#include "connectwindow.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QIntValidator>
#include <QDebug>

ConnectWindow::ConnectWindow(QWidget *parent)
    : QWidget(parent), m_socket(new QTcpSocket(this)), m_timeoutTimer(new QTimer(this))
{
    m_nameEdit = new QLineEdit;
    m_hostEdit = new QLineEdit("localhost");
    m_portEdit = new QLineEdit("12345");
    m_portEdit->setValidator(new QIntValidator(1, 65535, this));
    m_connectButton = new QPushButton("Отправить сообщение");
    m_statusLabel = new QLabel("Введите данные и нажмите кнопку");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Имя студента:"));
    layout->addWidget(m_nameEdit);
    layout->addWidget(new QLabel("Хост:"));
    layout->addWidget(m_hostEdit);
    layout->addWidget(new QLabel("Порт:"));
    layout->addWidget(m_portEdit);
    layout->addWidget(m_connectButton);
    layout->addWidget(m_statusLabel);
    setLayout(layout);
    setWindowTitle("Garson Client");

    connect(m_connectButton, &QPushButton::clicked, this, &ConnectWindow::onConnectClicked);
    connect(m_socket, &QTcpSocket::connected, this, &ConnectWindow::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ConnectWindow::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &ConnectWindow::onErrorOccurred);
    connect(m_timeoutTimer, &QTimer::timeout, this, &ConnectWindow::onTimeout);
}

void ConnectWindow::onConnectClicked()
{
    QString name = m_nameEdit->text().trimmed();
    QString host = m_hostEdit->text().trimmed();
    quint16 port = m_portEdit->text().toUShort();

    if (name.isEmpty()) {
        m_statusLabel->setText("Введите имя.");
        return;
    }

    m_statusLabel->setText("Подключение...");
    resetConnection();

    m_socket->connectToHost(host, port);
    m_timeoutTimer->start(10000); // 10 секунд таймаут
}

void ConnectWindow::onConnected()
{
    m_timeoutTimer->stop();
    QString name = m_nameEdit->text().trimmed();
    QString message = QString("Hello, Garson, I'm %1!\n").arg(name);
    m_socket->write(message.toUtf8());
    m_statusLabel->setText("Сообщение отправлено. Ожидаем ответ...");
}

void ConnectWindow::onReadyRead()
{
    m_timeoutTimer->stop();
    QByteArray response = m_socket->readAll();
    QString responseStr = QString::fromUtf8(response).trimmed();
    qDebug() << "Ответ от сервера:" << responseStr;

    if (responseStr.contains("Go To Sleep To the Garden!")) {
        QString name = m_nameEdit->text().trimmed();
        MainWindow *w = new MainWindow(name);
        w->show();
        this->hide();
    } else {
        QMessageBox::information(this, "Ответ от сервера", responseStr);
        m_statusLabel->setText("Ответ получен.");
    }
}

void ConnectWindow::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    m_timeoutTimer->stop();
    QMessageBox::critical(this, "Ошибка подключения", m_socket->errorString());
    m_statusLabel->setText("Ошибка подключения. Попробуйте снова.");
}

void ConnectWindow::onTimeout()
{
    m_socket->abort();
    QMessageBox::critical(this, "Ошибка подключения", "Превышено время ожидания подключения (10 секунд)");
    m_statusLabel->setText("Ошибка подключения. Попробуйте снова.");
}

void ConnectWindow::resetConnection()
{
    m_socket->abort();
    m_timeoutTimer->stop();
}
