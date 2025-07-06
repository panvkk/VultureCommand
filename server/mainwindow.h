#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>

#include "server.h"
#include "photodisplay.h"

class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);

private:
    void setupUI();
    void startServer();
    void logMessage(const QString& message);

    Server* server;
    PhotoDisplay* display;
    QLabel* statusLabel;
    QListWidget* logList;

private slots:
    void handleValidRequest(const QString& surname);
    void handleInvalidRequest(const QString& message, const QString& error); // Новый слот
};

#endif // MAINWINDOW_H
