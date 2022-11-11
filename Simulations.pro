QT       += core gui opengl multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

INCLUDEPATH += code
INCLUDEPATH += extlibs

VPATH += code

SOURCES += \
    code/camera.cpp \
    code/colliders.cpp \
    code/forces.cpp \
    code/glutils.cpp \
    code/glwidget.cpp \
    code/integrators.cpp \
    code/main.cpp \
    code/mainwindow.cpp \
    code/model.cpp \
    code/particlesystem.cpp \
    code/scenecloth.cpp \
    code/scenefountain.cpp \
    code/sceneop.cpp \
    code/sceneprojectiles.cpp \
    code/scenerope.cpp \
    code/scenesnowball.cpp \
    code/scenesph_watercube.cpp \
    code/widgetcloth.cpp \
    code/widgetfountain.cpp \
    code/widgetop.cpp \
    code/widgetprojectiles.cpp \
    code/widgetrope.cpp \
    code/widgetsnowball.cpp \
    code/widgetsph_watercube.cpp

HEADERS += \
    code/camera.h \
    code/cloth.h \
    code/colliders.h \
    code/defines.h \
    code/forces.h \
    code/glutils.h \
    code/glwidget.h \
    code/hash.h \
    code/integrators.h \
    code/mainwindow.h \
    code/model.h \
    code/particle.h \
    code/particlesystem.h \
    code/rope.h \
    code/sail.h \
    code/scene.h \
    code/scenecloth.h \
    code/scenefountain.h \
    code/sceneop.h \
    code/sceneprojectiles.h \
    code/scenerope.h \
    code/scenesnowball.h \
    code/scenesph_watercube.h \
    code/widgetcloth.h \
    code/widgetfountain.h \
    code/widgetop.h \
    code/widgetprojectiles.h \
    code/widgetrope.h \
    code/widgetsnowball.h \
    code/widgetsph_watercube.h

FORMS += \
    forms/mainwindow.ui \
    forms/widgetcloth.ui \
    forms/widgetfountain.ui \
    forms/widgetop.ui \
    forms/widgetprojectiles.ui \
    forms/widgetrope.ui \
    forms/widgetsnowball.ui \
    forms/widgetsph_watercube.ui

RESOURCES += shaders.qrc
