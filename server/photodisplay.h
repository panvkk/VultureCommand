#ifndef PHOTODISPLAY_H
#define PHOTODISPLAY_H

#include <QLabel>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

class PhotoDisplay : public QLabel {
    Q_OBJECT
public:
    explicit PhotoDisplay(QWidget* parent = nullptr);

public slots:
    void showPhoto(const QString& surname);

    // Добавляем приватный член для хранения указателя на анимацию
private:
    void jump();
    QPoint originalPos;
    QSequentialAnimationGroup* jumpAnimationGroup = nullptr;
};

#endif // PHOTODISPLAY_H
