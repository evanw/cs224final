QT       += core gui opengl
TARGET = cs224final
TEMPLATE = app
FORMS    += mainwindow.ui
INCLUDEPATH += ui doc util

HEADERS += \
    util/vector.h \
    ui/mainwindow.h \
    ui/camera.h \
    ui/view.h \
    util/geometry.h \
    ui/tools.h \
    doc/mesh.h \
    doc/document.h \
    lib/catmullclark.h

SOURCES += \
    ui/mainwindow.cpp \
    ui/main.cpp \
    ui/camera.cpp \
    ui/view.cpp \
    doc/document.cpp \
    util/geometry.cpp \
    ui/tools.cpp \
    doc/mesh.cpp \
    lib/catmullclark.cpp
