#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString& studentName, QWidget *parent = nullptr);

private:
    QLabel *m_label;
};

#endif // MAINWINDOW_H
