#-------------------------------------------------
#
# Project created by QtCreator 2019-12-12T11:31:14
#
#-------------------------------------------------

QT       += core gui serialport concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Sample
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        Lib/basescene3d.cpp \
        Lib/gl_primitives.cpp \
        Lib/varianteditor.cpp \
        main.cpp \
        window.cpp

HEADERS += \
        Lib/basescene3d.h \
        Lib/gl_primitives.h \
        Lib/varianteditor.h \
        window.h

FORMS += \
        Lib/basesettingswindow.ui \
        window.ui

win32 {
   LIBS += -lshell32 -lopengl32 -lwinmm -liphlpapi
} else {
    LIBS += -L"$(OutDir)/$(PLATFORM)" -lGLU
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    shaders.qrc
