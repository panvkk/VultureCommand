QT += core gui multimedia widgets
CONFIG += c++17

TARGET = laba3schitalki
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           person.cpp

HEADERS += mainwindow.h \
           person.h

RESOURCES += \
    resources.qrc \
    sounds.qrc \
    images.qrc

DISTFILES += \
    photos/BatrameevAndrey.jpg \
    photos/IsaevKirill.jpg \
    photos/KarpovichYulia.jpg \
    photos/KhaninStanislav.jpg \
    photos/MaslenchenkoKsenia.jpg \
    photos/PanovKonstantin.jpg \
    photos/StarovoytovIlya.jpg \
    photos/TrofimovStanislav.png \
    photos/VoroninaUliana.jpg

FORMS += \
    mainwindow.ui
