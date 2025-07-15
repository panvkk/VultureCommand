#include "mainwindow.h"
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

MainWindow::MainWindow(const QString& surname, QWidget *parent)
    : QMainWindow(parent), m_surname(surname)
{
    setWindowTitle("Сад для сна - " + m_surname);
    setFixedSize(800, 600);  // Фиксированный размер окна

    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);

    m_combinedLabel = new QLabel(this);
    createCombinedImage();

    layout->addWidget(m_combinedLabel);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}

void MainWindow::createCombinedImage()
{
    // 1. Пытаемся загрузить фото в разных форматах
    QPixmap studentPhoto;
    QString basePath = QCoreApplication::applicationDirPath() + "/photos/" + m_surname;

    // Сначала пробуем jpg, потом png
    if (!studentPhoto.load(basePath + ".jpg")) {
        studentPhoto.load(basePath + ".png");  // Если jpg не загрузилось, пробуем png
    }

    bool hasPhoto = !studentPhoto.isNull();

    if (hasPhoto) {
        studentPhoto = studentPhoto.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // 2. Создаем большое изображение сада (во все окно)
    QImage gardenImage(800, 600, QImage::Format_ARGB32);
    gardenImage.fill(QColor(144, 238, 144));  // Заливаем светлозеленым

    QPainter gardenPainter(&gardenImage);

    // Рисуем траву (нижняя часть)
    gardenPainter.setBrush(QColor(34, 139, 34));
    gardenPainter.drawRect(0, 500, 800, 100);  // Трава теперь внизу

    // Дерево (оставляем на том же месте относительно центра)
    gardenPainter.setBrush(QColor(139, 69, 19));
    gardenPainter.drawRect(350, 400, 40, 100);  // Ствол (центрирован)
    gardenPainter.setBrush(QColor(0, 100, 0));
    gardenPainter.drawEllipse(270, 300, 200, 150);  // Крона

    // Текст
    gardenPainter.setPen(Qt::black);
    gardenPainter.setFont(QFont("Arial", 16, QFont::Bold));
    gardenPainter.drawText(120, 320, "Zzz...");   // Текст у дерева

    // 3. Накладываем фото
    if (hasPhoto) {
        gardenPainter.drawPixmap(75, 340, studentPhoto);  // Фото в левом верхнем углу
    } else {
        gardenPainter.setPen(Qt::black);
        gardenPainter.setFont(QFont("Arial", 12));
        gardenPainter.drawText(QRect(50, 50, 200, 200), Qt::AlignCenter, "Фото\n" + m_surname);
    }

    m_combinedLabel->setPixmap(QPixmap::fromImage(gardenImage));
}
