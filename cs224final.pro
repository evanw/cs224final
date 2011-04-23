QT       += core gui opengl
TARGET = cs224final
TEMPLATE = app
FORMS    += mainwindow.ui
INCLUDEPATH += ui doc util b_mesh

HEADERS += \
    util/vector.h \
    ui/mainwindow.h \
    ui/camera.h \
    ui/view.h \
    util/geometry.h \
    ui/tools.h \
    doc/mesh.h \
    doc/document.h \
    b_mesh/meshconstruction.h \
    util/selectionrecorder.h \
    util/raytracer.h \
    b_mesh/catmullclark.h \
    doc/commands.h

SOURCES += \
    ui/mainwindow.cpp \
    ui/main.cpp \
    ui/camera.cpp \
    ui/view.cpp \
    doc/document.cpp \
    util/geometry.cpp \
    ui/tools.cpp \
    doc/mesh.cpp \
    b_mesh/meshconstruction.cpp \
    doc/objfileformat.cpp \
    util/selectionrecorder.cpp \
    util/raytracer.cpp \
    b_mesh/catmullclark.cpp \
    doc/commands.cpp
