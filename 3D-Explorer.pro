QT       += core gui 3d

TARGET = 3D-Explorer
TEMPLATE = app

RESOURCES += \
    3dexplorer.qrc

HEADERS += \
    view.h \
    imageobject.h \
    imageviewer.h \
    directory.h \
    pickobject.h \
    room.h \
    common.h \
    lib/glview.h \
    lib/qgldrawbuffersurface_p.h \
    lib/qglmaskedsurface_p.h

SOURCES += \
    main.cpp \
    view.cpp \
    imageobject.cpp \
    imageviewer.cpp \
    directory.cpp \
    pickobject.cpp \
    paint.cpp \
    control.cpp \
    room.cpp \
    animation.cpp \
    lib/glview.cpp \
    lib/qgldrawbuffersurface.cpp \
    lib/qglmaskedsurface.cpp \
    config.cpp

QMAKE_CXXFLAGS += -std=gnu++0x
