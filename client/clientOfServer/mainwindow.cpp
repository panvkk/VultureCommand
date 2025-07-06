#include "mainwindow.h"
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFile>
#include <QDir>

MainWindow::MainWindow(const QString& surname, QWidget *parent)
    : QMainWindow(parent), m_surname(surname)
{
    setWindowTitle("Сад для сна - " + m_surname);
    resize(800, 400);

    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);

    // Создаем изображение сада
    m_gardenLabel = new QLabel(this);
    createGardenImage();

    // Загружаем фото студента
    m_photoLabel = new QLabel(this);
    loadStudentPhoto();

    layout->addWidget(m_gardenLabel);
    layout->addWidget(m_photoLabel);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}

void MainWindow::createGardenImage()
{
    QImage gardenImage(400, 300, QImage::Format_RGB32);
    gardenImage.fill(QColor(144, 238, 144)); // Светло-зеленый фон

    QPainter painter(&gardenImage);

    // Рисуем траву
    painter.setBrush(QColor(34, 139, 34));
    painter.drawRect(0, 250, 400, 50);

    // Рисуем дерево
    painter.setBrush(QColor(139, 69, 19));
    painter.drawRect(180, 200, 40, 100); // Ствол
    painter.setBrush(QColor(0, 100, 0));
    painter.drawEllipse(100, 100, 200, 150); // Крона

    // Рисуем текст
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 14));
    painter.drawText(150, 280, m_surname);
    painter.drawText(170, 150, "Zzz...");

    m_gardenLabel->setPixmap(QPixmap::fromImage(gardenImage));
}

void MainWindow::loadStudentPhoto()
{
    QString photoPath = QDir::currentPath() + "/" + m_surname + ".jpg";

    if (QFile::exists(photoPath)) {
        QPixmap photo(photoPath);
        if (!photo.isNull()) {
            m_photoLabel->setPixmap(photo.scaled(300, 300, Qt::KeepAspectRatio));
            return;
        }
    }

    // Если фото не найдено, показываем заглушку
    QImage placeholder(300, 300, QImage::Format_RGB32);
    placeholder.fill(Qt::lightGray);

    QPainter p(&placeholder);
    p.setPen(Qt::black);
    p.setFont(QFont("Arial", 12));
    p.drawText(placeholder.rect(), Qt::AlignCenter, "Фото не найдено\n" + m_surname);

    m_photoLabel->setPixmap(QPixmap::fromImage(placeholder));
}
