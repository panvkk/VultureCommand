#include <QApplication>
#include <mainwindow.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    window.setFixedSize(750,750);
    window.setWindowTitle("VultureCommandIKM");
    return app.exec();
}

