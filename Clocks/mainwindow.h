#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QSoundEffect>
#include <QPixmap>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateClock();
    void playHourChime(int hour);

private:
    void drawClockFace(QPainter &painter);
    void drawHand(QPainter &painter, double angle, int length, int width, QColor color);
    void drawCuckoo(QPainter &painter, int secondsVisible);

    QPixmap background;
    QTimer *timer;
    int lastSecond;
    bool isTick;

    // Звуковые эффекты
    QSoundEffect tickSound;
    QSoundEffect tockSound;
    QSoundEffect boomSound;
    QSoundEffect cuckooSound;

    int lastChimedHour;
    bool cuckooPlayed;
    bool hourChimePlayed;
};
#endif // MAINWINDOW_H
