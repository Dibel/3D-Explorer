QT       += core gui 3d

TARGET = 3D-Explorer
TEMPLATE = app

RESOURCES += \
    3dexplorer.qrc

HEADERS += \
    meshobject.h \
    view.h \
    mainwindow.h

SOURCES += \
    main.cpp \
    meshobject.cpp \
    view.cpp \
    mainwindow.cpp
