#include <QApplication>
#include <QStyleHints>
#include <QSettings>
#include "mainwindow.h"

// Функция проверки темы Windows
bool isDarkTheme() {
    if (QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
        return true;
    }

    // Для версий Qt < 6.5
    QSettings registry(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat
        );
    return registry.value("AppsUseLightTheme", 1).toInt() == 0;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Определение темы при запуске
    bool darkTheme = isDarkTheme();

    // Установка глобальной палитры
    QPalette palette = QApplication::palette();
    if (darkTheme) {
        palette.setColor(QPalette::Window, QColor(45, 45, 45));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(35, 35, 35));
        palette.setColor(QPalette::Text, Qt::white);
    }
    QApplication::setPalette(palette);

    MainWindow window;
    window.show();
    window.setFixedSize(750,750);
    return app.exec();
}

