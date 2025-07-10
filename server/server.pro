QT += widgets network
TARGET = server
CONFIG += c++17

# Автоматическая генерация MOC
CONFIG += automoc

# Исходные файлы
SOURCES = \
    clienthandler.cpp \
    main.cpp \
    mainwindow.cpp \
    photodisplay.cpp \
    server.cpp

HEADERS = \
    clienthandler.h \
    mainwindow.h \
    photodisplay.h \
    server.h

# Явное указание на необходимость MOC
moc_output.source = moc
moc_output.input = HEADERS
moc_output.output_dir = debug
moc_output.dependency_type = TYPE_C
moc_output.variable_out = SOURCES
moc_output.name = MOC for ${QMAKE_FILE_IN}
moc_output.commands = $$QMAKE_MOC ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
moc_output.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += moc_output
