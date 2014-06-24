QT       += core gui 3d

TARGET = 3D-Explorer
TEMPLATE = app

RESOURCES += \
    3dexplorer.qrc

HEADERS += \
    meshobject.h \
    view.h \
    imageobject.h \
    directory.h \
    pickobject.h \
    room.h \
    common.h \
    lib/glview.h \
    lib/qgldrawbuffersurface_p.h \
    lib/qglmaskedsurface_p.h

SOURCES += \
    main.cpp \
    meshobject.cpp \
    view.cpp \
    imageobject.cpp \
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
