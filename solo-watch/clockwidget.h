#ifndef CLOCKWIDGET_H
#define CLOCKWIDGET_H

#include <QWidget>
#include <QTime>
#include <QPixmap>
#include <QPointF>
#include <QPropertyAnimation>

class ClockWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int owlY READ getOwlY WRITE setOwlY)

public:
    explicit ClockWidget(QWidget *parent = nullptr);
    void setTime(const QTime &time);
    void startOwlAnimation();
    int getOwlY() const { return owlY; }
    void setOwlY(int y);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawClockFace(QPainter &painter);
    void drawHand(QPainter &painter, const QPixmap &hand, float rotation, const QPointF &pivot);
    void loadImages();
    void scaleImages();

    QTime currentTime;

    QPixmap background;
    QPixmap clockFace;
    QPixmap hourHand, minuteHand, secondHand;
    QPixmap owlImage;

    QPixmap scaledBackground, scaledClockFace;
    QPixmap scaledHourHand, scaledMinuteHand, scaledSecondHand;
    QPixmap scaledOwlImage;

    QSize originalSize;

    // Owl animation
    int owlY;
    bool isOwlVisible;
    QPropertyAnimation *owlAnimation;
};

#endif // CLOCKWIDGET_H
