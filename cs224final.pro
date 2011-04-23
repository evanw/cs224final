QT       += core gui opengl
TARGET = cs224final
TEMPLATE = app
FORMS    += mainwindow.ui
INCLUDEPATH += ui doc util

HEADERS += \
    util/vector.h \
    doc/rawdocument.h \
    ui/mainwindow.h \
    ui/camera.h \
    ui/view.h \
    doc/document.h \
    util/geometry.h \
    ui/tools.h

SOURCES += \
    doc/rawdocument.cpp \
    ui/mainwindow.cpp \
    ui/main.cpp \
    ui/camera.cpp \
    ui/view.cpp \
    doc/document.cpp \
    util/geometry.cpp \
    ui/tools.cpp
