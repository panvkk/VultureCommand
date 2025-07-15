#include "clockwidget.h"
#include <QPainter>
#include <QResizeEvent>
#include <QtMath>
#include <QDebug>
#include <QEasingCurve>

ClockWidget::ClockWidget(QWidget *parent)
    : QWidget(parent), owlY(100), isOwlVisible(false), originalSize(1920, 1080)
{
    loadImages();

    owlAnimation = new QPropertyAnimation(this, "owlY", this);
    owlAnimation->setDuration(19000);
    owlAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(owlAnimation, &QPropertyAnimation::finished, this, &ClockWidget::onOwlAnimationFinished);
}

void ClockWidget::loadImages() {
    background = QPixmap(":/images/background.jpg");
    clockFace = QPixmap(":/images/clock_face.png");
    hourHand = QPixmap(":/images/hour_hand.png");
    minuteHand = QPixmap(":/images/minute_hand.png");
    secondHand = QPixmap(":/images/second_hand.png");
    owlImage = QPixmap(":/images/SOVA.png");

    originalSize = background.size();
    scaleImages();
}

void ClockWidget::scaleImages() {
    scaledBackground = background.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    int clockSize = qMin(width(), height()) * 0.85;

    scaledClockFace = clockFace.scaled(clockSize, clockSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    double handScale = clockSize / 500.0;

    scaledHourHand = hourHand.scaled(hourHand.size() * handScale * 0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledMinuteHand = minuteHand.scaled(minuteHand.size() * handScale * 0.9, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledSecondHand = secondHand.scaled(secondHand.size() * handScale, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Сова масштабируется относительно ширины окна
    scaledOwlImage = owlImage.scaled(width() * 0.3, owlImage.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void ClockWidget::setTime(const QTime &time) {
    currentTime = time;
    update();
}

void ClockWidget::resizeEvent(QResizeEvent *) {
    scaleImages();
}

void ClockWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.drawPixmap(rect(), scaledBackground);

    drawClockFace(painter);

    float hourAngle = 180 + 30 * (currentTime.hour() % 12) + 0.5 * currentTime.minute();
    float minuteAngle = 180 + 6 * currentTime.minute() + 0.1 * currentTime.second();
    float secondAngle = 180 + 6 * currentTime.second();

    QPointF hourPivot(0, scaledHourHand.height() / 2);
    QPointF minutePivot(0, scaledMinuteHand.height() / 2);
    QPointF secondPivot(0, scaledSecondHand.height() / 2);

    drawHand(painter, scaledHourHand, hourAngle, hourPivot);
    drawHand(painter, scaledMinuteHand, minuteAngle, minutePivot);
    drawHand(painter, scaledSecondHand, secondAngle, secondPivot);

    if (isOwlVisible) {
        int owlX = (width() - scaledOwlImage.width()) / 2;
        painter.drawPixmap(owlX, owlY, scaledOwlImage);
        qDebug() << "🎨 Рисуем сову на координатах X:" << owlX << "Y:" << owlY;
    }
}

void ClockWidget::drawClockFace(QPainter &painter) {
    int x = (width() - scaledClockFace.width()) / 2;
    int y = (height() - scaledClockFace.height()) / 2;
    painter.drawPixmap(x, y, scaledClockFace);
}

void ClockWidget::drawHand(QPainter &painter, const QPixmap &hand, float rotation, const QPointF &pivot) {
    int centerX = width() / 2;
    int centerY = height() / 2;

    painter.save();
    painter.translate(centerX, centerY);
    painter.rotate(rotation);

    int offsetX = -hand.width() / 2 + pivot.x();
    int offsetY = -hand.height() / 2 + pivot.y();

    painter.drawPixmap(offsetX, offsetY, hand);
    painter.restore();
}

void ClockWidget::startOwlAnimation() {
    isOwlVisible = true;
    owlY = -scaledOwlImage.height();
    update();

    owlAnimation->setStartValue(owlY);
    owlAnimation->setEndValue(height() / 2);
    owlAnimation->start();
}

void ClockWidget::setOwlY(int y) {
    owlY = y;
    update();
}

void ClockWidget::onOwlAnimationFinished() {
    owlAnimation->setStartValue(owlY);
    owlAnimation->setEndValue(-scaledOwlImage.height());
    owlAnimation->start();
}
