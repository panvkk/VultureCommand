QT += core gui multimedia widgets
CONFIG += c++17

TARGET = laba3schitalki
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           person.cpp \

HEADERS += mainwindow.h \
           person.h \

RESOURCES += \
    resources.qrc \
    sounds.qrc \
    images.qrc

FORMS += \
    mainwindow.ui
