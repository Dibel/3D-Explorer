QT       += core gui 3d

TARGET = 3D-Explorer
TEMPLATE = app

RESOURCES += \
    3dexplorer.qrc

HEADERS += \
    meshobject.h \
    view.h \
    imageobject.h \
    directory.h

SOURCES += \
    main.cpp \
    meshobject.cpp \
    view.cpp \
    imageobject.cpp \
    directory.cpp

QMAKE_CXXFLAGS += -std=gnu++0x
