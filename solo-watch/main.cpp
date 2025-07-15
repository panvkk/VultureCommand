#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Регистрация ресурсов
    Q_INIT_RESOURCE(resources);

    MainWindow window;
    window.show();

    return app.exec();
}
