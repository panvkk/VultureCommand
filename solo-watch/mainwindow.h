#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QSoundEffect>
#include "clockwidget.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateClock();

private:
    void setupSounds();
    void setupUI();

    ClockWidget *clockWidget;
    QTimer *timer;

    QSoundEffect tickSound;
    QSoundEffect tackSound;
    QSoundEffect boomSound;
    QSoundEffect cuckooSound;

    bool isTick;
    bool isStriking;
    int strikeCount;
    int strikeStep;
};

#endif // MAINWINDOW_H
