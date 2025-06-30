#include <QApplication>
#include <mainwindow.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    window.setFixedSize(750,750);
    return app.exec();
}
