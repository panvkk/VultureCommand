QT += core gui multimedia widgets
CONFIG += c++17

TARGET = laba3schitalki
TEMPLATE = app

SOURCES += main.cpp \
    client.cpp \
    connectwindow.cpp \
           mainwindow.cpp \
           person.cpp \
    report.cpp \
    staticticsmanager.cpp \
    waitingwindow.cpp \
    winnerReport.cpp

HEADERS += mainwindow.h \
    client.h \
    connectwindow.h \
           person.h \
    report.h \
    statisticsmanager.h \
    waitingwindow.h \
    winnerReport.h

RESOURCES += \
    resources.qrc \
    sounds.qrc \
    images.qrc

FORMS += \
    mainwindow.ui
