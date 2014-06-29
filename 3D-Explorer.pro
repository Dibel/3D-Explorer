QT       += core gui 3d multimedia

TARGET = 3D-Explorer
TEMPLATE = app

RESOURCES += \
    3dexplorer.qrc

HEADERS += \
    view.h \
    outlinepainter.h \
    imageobject.h \
    imageviewer.h \
    directory.h \
    pickobject.h \
    room.h \
    common.h \
    lib/glview.h \
    lib/gldrawbuffersurface_p.h \
    lib/glmaskedsurface_p.h

SOURCES += \
    main.cpp \
    view.cpp \
    outlinepainter.cpp \
    imageobject.cpp \
    imageviewer.cpp \
    directory.cpp \
    pickobject.cpp \
    paint.cpp \
    control.cpp \
    room.cpp \
    animation.cpp \
    lib/glview.cpp \
    lib/gldrawbuffersurface.cpp \
    lib/glmaskedsurface.cpp \
    config.cpp

QMAKE_CXXFLAGS += -std=gnu++0x
