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
    m_surnameEdit = new QLineEdit;
    m_ipEdit = new QLineEdit("10.160.0.149");
    m_portEdit = new QLineEdit("8080");
    m_portEdit->setValidator(new QIntValidator(1, 65535, this));
    m_messageEdit = new QLineEdit;
    m_messageEdit->setPlaceholderText("Введите сообщение для сервера");
    m_connectButton = new QPushButton("Отправить сообщение");
    m_statusLabel = new QLabel("Введите данные и нажмите кнопку");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Фамилия студента:"));
    layout->addWidget(m_surnameEdit);
    layout->addWidget(new QLabel("IP-адрес сервера:"));
    layout->addWidget(m_ipEdit);
    layout->addWidget(new QLabel("Порт сервера:"));
    layout->addWidget(m_portEdit);
    layout->addWidget(new QLabel("Сообщение (необязательно):"));
    layout->addWidget(m_messageEdit);
    layout->addWidget(m_connectButton);
    layout->addWidget(m_statusLabel);
    setLayout(layout);
    setWindowTitle("VultureCommandIKM");
    connect(m_connectButton, &QPushButton::clicked, this, &ConnectWindow::onConnectClicked);
    connect(m_socket, &QTcpSocket::connected, this, &ConnectWindow::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ConnectWindow::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &ConnectWindow::onErrorOccurred);
    connect(m_timeoutTimer, &QTimer::timeout, this, &ConnectWindow::onTimeout);
}

void ConnectWindow::onConnectClicked()
{
    QString surname = m_surnameEdit->text().trimmed();
    QString ip = m_ipEdit->text().trimmed();
    quint16 port = m_portEdit->text().toUShort();
    QString message = m_messageEdit->text().trimmed();

    if (surname.isEmpty()) {
        m_statusLabel->setText("Ошибка: введите фамилию!");
        return;
    }

    // Проверка на соответствие фамилий
    if (!message.isEmpty() && message.contains("I'm") && !message.contains(surname, Qt::CaseInsensitive)) {
        m_statusLabel->setText("Обманщик! Ты кто вообще? ОПРЕДЕЛИСЬ с фамилией!!!");
        return;
    }

    m_statusLabel->setText("Подключаемся к серверу...");
    resetConnection();

    m_socket->connectToHost(ip, port);
    m_timeoutTimer->start(10000); // Таймаут 10 секунд
}


void ConnectWindow::onConnected()
{
    m_timeoutTimer->stop();
    if(m_socket->state() != QTcpSocket::ConnectedState) {
        qDebug() << "Ошибка: соединение не установлено";
        return;
    }

    QString surname = m_surnameEdit->text().trimmed();
    QString message = m_messageEdit->text().trimmed() + "\n";
    if(message == "\n") message = QString("Hello, Garson, I'm %1!\n").arg(surname);

    /*qint64 bytesWritten = m_socket->write(message.toUtf8());
    if(bytesWritten == -1) {
        qDebug() << "Ошибка отправки:" << m_socket->errorString();
    } else {
        qDebug() << "Отправлено" << bytesWritten << "байт";
    }*/
    m_socket->write(message.toUtf8());
    m_statusLabel->setText("Сообщение отправлено. Ждем ответ...");
}

void ConnectWindow::onReadyRead()
{
    m_timeoutTimer->stop();
    QByteArray response = m_socket->readAll();
    QString responseStr = QString::fromUtf8(response).trimmed();
    qDebug() << "Ответ сервера:" << responseStr;

    if (responseStr.contains("Go To Sleep To the Garden!")) {
        QString surname = m_surnameEdit->text().trimmed();
        MainWindow *w = new MainWindow(surname);
        w->show();
        this->hide();
    } else {
        QMessageBox::information(this, "Ответ сервера", responseStr);
        m_statusLabel->setText("Ответ получен: " + responseStr);
    }
}

void ConnectWindow::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    m_timeoutTimer->stop();
    //QMessageBox::critical(this, "Ошибка подключения", m_socket->errorString());
    m_statusLabel->setText("Ошибка: " + m_socket->errorString());
}

void ConnectWindow::onTimeout()
{
    m_socket->abort();
    QMessageBox::critical(this, "Ошибка", "Превышено время ожидания подключения!");
    m_statusLabel->setText("Ошибка: таймаут подключения");
}

void ConnectWindow::resetConnection()
{
    m_socket->abort();
    m_timeoutTimer->stop();
}
