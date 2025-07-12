#include "mainwindow.h"
#include <QVBoxLayout>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
    isTick(true),
    isStriking(false),
    strikeCount(0),
    strikeStep(0)
{
    setupUI();
    setupSounds();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateClock);
    timer->start(100);
}

void MainWindow::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    clockWidget = new ClockWidget(this);
    clockWidget->setMinimumSize(500, 500);
    layout->addWidget(clockWidget);
    setLayout(layout);

    setWindowTitle("WatchesIKM");
    resize(600, 600);
}

void MainWindow::setupSounds() {
    tickSound.setSource(QUrl("qrc:/sounds/tick.wav"));
    tackSound.setSource(QUrl("qrc:/sounds/tack.wav"));
    boomSound.setSource(QUrl("qrc:/sounds/boom.wav"));
    cuckooSound.setSource(QUrl("qrc:/sounds/cuckoo.wav"));

    tickSound.setVolume(0.4f);
    tackSound.setVolume(0.4f);
    boomSound.setVolume(0.9f);
    cuckooSound.setVolume(0.8f);
}

void MainWindow::updateClock() {
    QTime now = QTime::currentTime();
    clockWidget->setTime(now);

    if (now.minute() == 0 && now.second() == 0 && !isStriking) {
        int hour = now.hour();
        strikeCount = (hour == 0 || hour == 12) ? 12 : hour % 12;
        isStriking = strikeCount > 0;
        strikeStep = 0;

        qDebug() << "🔔 Новый час! Часов: " << strikeCount;

        if (hour == 0 || hour == 12) {
            clockWidget->startOwlAnimation();
        }
    }

    if (isStriking) {
        if (strikeStep % 20 == 0) {
            qDebug() << "🔊 Удар часов. Осталось ударов:" << strikeCount;
            int hour = QTime::currentTime().hour();
            (hour == 0 || hour == 12) ? cuckooSound.play() : boomSound.play();
            strikeCount--;
            if (strikeCount == 0) isStriking = false;
        }
        strikeStep++;
    }

    if (now.msec() < 100) {
        isTick ? tickSound.play() : tackSound.play();
        isTick = !isTick;
    }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    // Ничего не рисуем вручную — всё отображает ClockWidget
}
