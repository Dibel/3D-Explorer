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
    common.h

SOURCES += \
    main.cpp \
    meshobject.cpp \
    view.cpp \
    imageobject.cpp \
    directory.cpp \
    pickobject.cpp \
    paint.cpp \
    control.cpp \
    loadmodel.cpp \
    room.cpp

QMAKE_CXXFLAGS += -std=gnu++0x
