#include "photodisplay.h"
#include <QPixmap>
#include <QFile>
#include <QTimer>
#include <QDir>

PhotoDisplay::PhotoDisplay(QWidget* parent) : QLabel(parent) {
    setAlignment(Qt::AlignCenter);
    setMinimumSize(400, 400);
    setStyleSheet("background-color: black; color: white;");
    setText("Waiting for students...");
}

void PhotoDisplay::showPhoto(const QString& surname) {
    QString path = QString("photos/%1.jpg").arg(surname);

    if(QFile::exists(path)) {
        QPixmap pixmap(path);
        if(!pixmap.isNull()) {
            setPixmap(pixmap.scaled(400, 400, Qt::KeepAspectRatio));
            QTimer::singleShot(5000, this, [this]{
                setPixmap(QPixmap());
                setText("Waiting for students...");
            });
        } else {
            setText("Invalid image format");
            QTimer::singleShot(5000, this, [this]{
                setText("Waiting for students...");
            });
        }
    } else {
        setText(QString("Photo not found: %1").arg(surname));
        QTimer::singleShot(5000, this, [this]{
            setText("Waiting for students...");
        });
    }
}
