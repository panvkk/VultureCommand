#include <QApplication>
#include "connectwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ConnectWindow w;
    w.show();

    return a.exec();
}

