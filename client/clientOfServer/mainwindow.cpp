#include "mainwindow.h"
#include <QImage>
#include <QPainter>

MainWindow::MainWindow(const QString& studentName, QWidget *parent)
    : QMainWindow(parent), m_label(new QLabel(this))
{
    setCentralWidget(m_label);
    setWindowTitle("Спим в саду");

    QImage image(400, 300, QImage::Format_RGB32);
    image.fill(Qt::green);

    QPainter painter(&image);
    painter.setBrush(Qt::blue);
    painter.drawEllipse(150, 100, 100, 100);
    painter.drawRect(175, 200, 50, 100);
    painter.drawText(160, 150, "Zzz...");
    painter.drawText(100, 250, studentName);

    m_label->setPixmap(QPixmap::fromImage(image));
    resize(400, 300);
}
