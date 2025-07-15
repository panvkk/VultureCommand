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
    void createCombinedImage();  // Изменили название метода
    void loadStudentPhoto();    // Этот метод теперь используется внутри createCombinedImage

    QLabel *m_combinedLabel;   // Заменили два QLabel на один
    QString m_surname;
};

#endif // MAINWINDOW_H
