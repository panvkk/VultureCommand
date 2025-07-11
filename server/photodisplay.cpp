#include "photodisplay.h"
#include <QPixmap>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

PhotoDisplay::PhotoDisplay(QWidget* parent) : QLabel(parent) {
    setAlignment(Qt::AlignCenter);
    setMinimumSize(400, 400);
    setStyleSheet("background-color: black; color: white;");
    setText("Waiting for students...");
}

void PhotoDisplay::jump() {
    if (originalPos.isNull()) {
        originalPos = this->pos();
    }

    QPoint startPos = originalPos;
    QPoint jumpPos = QPoint(startPos.x(), startPos.y() - 30);

    QPropertyAnimation* animUp = new QPropertyAnimation(this, "pos");
    animUp->setDuration(200);
    animUp->setStartValue(startPos);
    animUp->setEndValue(jumpPos);
    animUp->setEasingCurve(QEasingCurve::OutQuad);

    QPropertyAnimation* animDown = new QPropertyAnimation(this, "pos");
    animDown->setDuration(200);
    animDown->setStartValue(jumpPos);
    animDown->setEndValue(startPos);
    animDown->setEasingCurve(QEasingCurve::InQuad);

    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this); // Указываем родителя для автоматического удаления
    group->addAnimation(animUp);
    group->addAnimation(animDown);
    group->setLoopCount(-1); // Запускаем анимацию в бесконечном цикле

    group->start();
}

void PhotoDisplay::showPhoto(const QString& surname) {
    QString appPath = QCoreApplication::applicationDirPath();
    QString path = QDir(appPath).filePath(QString("photos/%1.jpg").arg(surname));

    qDebug() << "Trying to load photo from:" << path;

    if (QFile::exists(path)) {
        QPixmap pixmap(path);
        if (!pixmap.isNull()) {
            setPixmap(pixmap.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            if (originalPos.isNull()) {
                originalPos = this->pos();
            }
            jump(); // Запускаем анимацию прыжка при показе нового фото
        } else {
            setText("Invalid image format");
            // Если фото некорректное, останавливаем прыжки (если они были)
            if (jumpAnimationGroup && jumpAnimationGroup->state() == QAbstractAnimation::Running) {
                jumpAnimationGroup->stop();
            }
        }
    } else {
        setText(QString("Photo not found: %1").arg(surname));
        // Если фото не найдено, останавливаем прыжки (если они были)
        if (jumpAnimationGroup && jumpAnimationGroup->state() == QAbstractAnimation::Running) {
            jumpAnimationGroup->stop();
        }
    }
}
