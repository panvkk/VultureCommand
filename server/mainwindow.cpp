#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QDateTime>
#include <QDir>
#include <QListWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUI();
    startServer();
}

void MainWindow::setupUI() {
    setWindowTitle("Group 7 Server");
    resize(800, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(statusLabel);

    display = new PhotoDisplay(this);
    mainLayout->addWidget(display, 1);

    logList = new QListWidget(this);
    logList->setStyleSheet("font-family: monospace;");
    mainLayout->addWidget(logList);

    QDir().mkdir("photos");
}

void MainWindow::startServer() {
    server = new Server(this);
    if(!server->listen(QHostAddress::Any, 8080)) {
        QMessageBox::critical(this, "Error", "Could not start server!");
        return;
    }

    QString ipAddress;
    const auto ipAddresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : ipAddresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            address != QHostAddress::LocalHost) {
            ipAddress = address.toString();
            break;
        }
    }
    if (ipAddress.isEmpty())
        ipAddress = "127.0.0.1";

    statusLabel->setText(QString("Server running on %1:%2").arg(ipAddress).arg(server->serverPort()));
    logMessage("Server started. Waiting for connections...");

    connect(server, &Server::validRequest, this, &MainWindow::handleValidRequest);
    connect(server, &Server::invalidRequest, this, &MainWindow::handleInvalidRequest); // Новое соединение
}

void MainWindow::handleValidRequest(const QString& surname) {


    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logMsg = QString("[%1] ✓ Correct request from: %2").arg(timestamp).arg(surname);
    logList->addItem(logMsg);

    // QListWidgetItem* item = new QListWidgetItem(logMsg);
    // item->setForeground(Qt::darkGreen);
    // logList->addItem(item);

    logList->scrollToBottom();

    display->showPhoto(surname);
}

// Новый метод для обработки неверных запросов
void MainWindow::handleInvalidRequest(const QString& message, const QString& error) {

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logMsg = QString("[%1] ✗ Invalid request: '%2' (%3)").arg(timestamp).arg(message).arg(error);
    logList->addItem(logMsg);

    // QListWidgetItem* item = new QListWidgetItem(logMsg);
    // item->setForeground(Qt::darkRed);
    // logList->addItem(item);

    logList->scrollToBottom();
}

void MainWindow::logMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    logList->addItem(QString("[%1] %2").arg(timestamp).arg(message));
    logList->scrollToBottom();
}
