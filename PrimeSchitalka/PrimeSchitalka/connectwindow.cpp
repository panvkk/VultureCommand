#include "connectwindow.h"
#include "waitingwindow.h"
#include "mainwindow.h"
#include <QIntValidator>
#include <QMessageBox>

ConnectWindow::ConnectWindow(QWidget *parent) : QWidget(parent), m_client(new Client(this))
{
    setupUi();
}

void ConnectWindow::onCountingRequested(int participants, const QString& rhyme)
{
    if (m_waitingWindow) {
        m_waitingWindow->hide();
        m_waitingWindow->deleteLater();
        m_waitingWindow = nullptr;
    }

    // Создаем главное окно с параметрами от сервера
    MainWindow *mainWindow = new MainWindow(participants, rhyme, m_nameEdit->text());

    // Подключаем сигнал завершения подсчета
    connect(mainWindow, &MainWindow::countingFinished, this, [this, mainWindow](const WinnerReport& report) {
        m_client->sendCountingResult(report);
        mainWindow->deleteLater();
        this->show();
    });

    mainWindow->show();
}

// Обработка нажатия кнопки подключения
// В connectwindow.cpp
void ConnectWindow::onConnectClicked()
{
    QString name = m_nameEdit->text().trimmed();
    QString host = m_hostEdit->text().trimmed();
    quint16 port = m_portEdit->text().toUShort();

    if (name.isEmpty()) {
        m_statusLabel->setText("Введите имя клиента");
        return;
    }

    // Показываем окно ожидания сразу при попытке подключения
    WaitingWindow* waitingWindow = new WaitingWindow(this);
    waitingWindow->show();
    this->hide();

    // Сохраняем ссылку на окно ожидания
    m_waitingWindow = waitingWindow;

    m_statusLabel->setText("Подключение к серверу...");
    m_client->connectToServer(host, port, name);

    // Подключаем сигналы клиента к слотам этого окна
    connect(m_client, &Client::countingRequested, this, &ConnectWindow::onCountingRequested);
    connect(m_client, &Client::verificationReceived, this, &ConnectWindow::onVerificationReceived);
    connect(m_client, &Client::statusMessage, this, &ConnectWindow::onStatusMessage);
    connect(m_client, &Client::connectionError, this, &ConnectWindow::onConnectionError);
}

// Обработка статусных сообщений
void ConnectWindow::onStatusMessage(const QString& message)
{
    m_statusLabel->setText(message);
}

// Обработка ошибок подключения
void ConnectWindow::onConnectionError(const QString& error)
{
    QMessageBox::critical(this, "Ошибка подключения", error);
    m_statusLabel->setText("Ошибка: " + error);
}

void ConnectWindow::setupUi()
{
    m_nameEdit = new QLineEdit(this);
    m_hostEdit = new QLineEdit("localhost", this);
    m_portEdit = new QLineEdit("12345", this);
    m_portEdit->setValidator(new QIntValidator(1, 65535, this));

    m_connectButton = new QPushButton("Подключиться", this);
    m_statusLabel = new QLabel("Введите данные для подключения", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Имя клиента:", this));
    layout->addWidget(m_nameEdit);
    layout->addWidget(new QLabel("Адрес сервера:", this));
    layout->addWidget(m_hostEdit);
    layout->addWidget(new QLabel("Порт сервера:", this));
    layout->addWidget(m_portEdit);
    layout->addWidget(m_connectButton);
    layout->addWidget(m_statusLabel);

    setLayout(layout);
    setWindowTitle("Клиент считалки");
}

void ConnectWindow::onVerificationReceived(bool isOk)
{
    if (isOk) {
        QMessageBox::information(this, "Результат", "Сервер подтвердил правильность выполнения!");
    } else {
        QMessageBox::warning(this, "Результат", "Сервер обнаружил расхождение в результатах!");
    }
    this->show();
}
