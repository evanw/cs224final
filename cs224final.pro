QT       += core gui opengl
TARGET = cs224final
TEMPLATE = app
FORMS    += mainwindow.ui
INCLUDEPATH += ui doc util b_mesh
LIBS += ../cs224final/lib/libCGAL.so

# needed for linking with CGAL
QMAKE_CXXFLAGS += -frounding-math

# set rpath to avoid dealing with LD_LIBRARY_PATH
unix:{
  QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/lib
  QMAKE_RPATH=
}

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
    doc/commands.h \
    b_mesh/meshevolution.h \
    util/convexhullsolver.h

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
    doc/commands.cpp \
    b_mesh/meshevolution.cpp \
    util/convexhullsolver.cpp

RESOURCES += \
    resources.qrc
