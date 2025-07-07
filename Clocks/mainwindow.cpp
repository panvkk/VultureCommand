#include "mainwindow.h"
#include <QPainter>
#include <QDebug>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setFixedSize(800, 600);
    // Загрузка фонового изображения
    if(!background.load(":/images/dali_background.jpg")) {
        background = QPixmap(800, 600);
        background.fill(Qt::black);
    }
    background = background.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    setFixedSize(background.size());

    // Настройка звуков
    tickSound.setSource(QUrl("qrc:/sounds/tick.wav"));
    tockSound.setSource(QUrl("qrc:/sounds/tack.wav"));
    boomSound.setSource(QUrl("qrc:/sounds/boom.wav"));
    cuckooSound.setSource(QUrl("qrc:/sounds/cuckoo.wav"));

    // Настройка таймера
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateClock);
    timer->start(1000);

    lastSecond = QTime::currentTime().second();
    isTick = true;

    lastChimedHour = -1; // Инициализируем недопустимым значением
    cuckooPlayed = false;
    hourChimePlayed = false;
}

MainWindow::~MainWindow()
{
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // Фон
    painter.drawPixmap(0, 0, background);

    QTime time = QTime::currentTime();
    if ((time.hour() == 12 || time.hour() == 0) &&
        time.minute() == 0 &&
        time.second() < 30 &&
        cuckooPlayed) {
        drawCuckoo(painter, time.second());
    }
    drawClockFace(painter);

    // Стрелки
    drawHand(painter, time.hour() * 30 + time.minute() * 0.5, 50, 6, Qt::black);
    drawHand(painter, time.minute() * 6, 70, 4, Qt::blue);
    drawHand(painter, time.second() * 6, 80, 2, Qt::red);

    // Кукушка в полдень/полночь
    if ((time.hour() == 12 || time.hour() == 0) &&
        time.minute() == 0 && time.second() < 30) {
        drawCuckoo(painter, time.second());
    }

}

void MainWindow::updateClock()
{
    QTime currentTime = QTime::currentTime();
    int currentSecond = currentTime.second();
    int currentHour = currentTime.hour();
    int currentMinute = currentTime.minute();

    // Тик-так каждую секунду
    if (currentSecond != lastSecond) {
        if (isTick) tickSound.play();
        else tockSound.play();
        isTick = !isTick;
        lastSecond = currentSecond;
    }

    // Сбрасываем флаги в начале нового часа
    if (currentMinute == 0 && currentSecond == 0) {
        if (currentHour != lastChimedHour) {
            cuckooPlayed = false;
            hourChimePlayed = false;
            lastChimedHour = currentHour;
        }
    }

    // Проигрываем кукушку или бой часов только если еще не проигрывали в этот час
    if (currentMinute == 0 && currentSecond == 0) {
        if (!hourChimePlayed && !cuckooPlayed) {
            playHourChime(currentHour);

            // Устанавливаем флаги
            if (currentHour == 12 || currentHour == 0) {
                cuckooPlayed = true;
            } else {
                hourChimePlayed = true;
            }
        }
    }

    update();
}

void MainWindow::playHourChime(int hour)
{
    // В полдень и полночь играем песню кукушки
    if (hour == 12 || hour == 0) {
        if (!cuckooPlayed) {
            // Кукушка 12 раз с интервалом 0.5 секунды
            for (int i = 0; i < 12; ++i) {
                QTimer::singleShot(i * 500, [this]() {
                    cuckooSound.play();
                });
            }
            cuckooPlayed = true;
        }
    }
    // В остальные часы - бой часов
    else {
        if (!hourChimePlayed) {
            int strikes = hour > 12 ? hour - 12 : hour;
            for (int i = 0; i < strikes; ++i) {
                QTimer::singleShot(i * 1000, [this]() {
                    boomSound.play();
                });
            }
            hourChimePlayed = true;
        }
    }
}

void MainWindow::drawClockFace(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);

    int centerX = width() / 2;
    int centerY = height() / 2;
    int radius = qMin(width(), height()) / 3;

    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::white);
    painter.drawEllipse(QPoint(centerX, centerY), radius, radius);

    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(12);
    painter.setFont(font);

    for (int i = 1; i <= 12; ++i) {
        double angle = (i * 30) * M_PI / 180;
        int x = centerX + (radius - 20) * sin(angle);
        int y = centerY - (radius - 20) * cos(angle);
        painter.drawText(x - 10, y - 10, 20, 20, Qt::AlignCenter, QString::number(i));
    }

    painter.restore();
}

void MainWindow::drawHand(QPainter &painter, double angle, int length, int width, QColor color)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);

    int centerX = background.width() / 2;
    int centerY = background.height() / 2;
    angle = (angle - 90) * M_PI / 180;
    int x = centerX + length * cos(angle);
    int y = centerY + length * sin(angle);

    painter.setPen(QPen(color, width));
    painter.drawLine(centerX, centerY, x, y);

    painter.restore();
}

void MainWindow::drawCuckoo(QPainter &painter, int secondsVisible)
{
    painter.save();

    int centerX = width() / 2;
    int centerY = height() / 2;
    int radius = qMin(width(), height()) / 3;
    int cuckooX = centerX + radius + 10 - qMin(secondsVisible * 5, 50);
    int cuckooY = centerY - 20;

    // Дверца
    painter.setPen(QPen(Qt::darkGreen, 2));
    painter.setBrush(Qt::lightGray);
    painter.drawRect(centerX + radius + 5, cuckooY - 30, 60, 40);

    // Кукушка
    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    painter.drawEllipse(cuckooX, cuckooY, 40, 30);
    painter.drawEllipse(cuckooX + 25, cuckooY - 10, 15, 15);
    painter.drawEllipse(cuckooX + 32, cuckooY - 8, 5, 5);
    painter.drawLine(cuckooX + 30, cuckooY + 5, cuckooX + 20, cuckooY + 15);

    painter.restore();
}
