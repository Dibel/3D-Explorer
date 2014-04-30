QT       += core gui 3d

TARGET = 3D-Explorer
TEMPLATE = app

RESOURCES += \
    3dexplorer.qrc

HEADERS += \
    meshobject.h \
    view.h \
    imageobject.h

SOURCES += \
    main.cpp \
    meshobject.cpp \
    view.cpp \
    imageobject.cpp

QMAKE_CXXFLAGS += -std=gnu++0x
