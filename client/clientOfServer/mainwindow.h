#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString& surname, QWidget *parent = nullptr);

private:
    void createGardenImage();
    void loadStudentPhoto();

    QLabel *m_gardenLabel;
    QLabel *m_photoLabel;
    QString m_surname;
};

#endif // MAINWINDOW_H
