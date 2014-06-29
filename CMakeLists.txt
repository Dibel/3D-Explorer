cmake_minimum_required(VERSION 2.8.11)

set(PROJECTNAME 3dexplorer)

set(SOURCE
    main.cpp
    config.cpp
    directory.cpp
    view.cpp
    paint.cpp
    control.cpp
    animation.cpp
    room.cpp
    imageviewer.cpp
    outlinepainter.cpp
    lib/glview.cpp
    lib/gldrawbuffersurface.cpp
    lib/glmaskedsurface.cpp
)

set(HEADER
    common.h
    directory.h
    view.h
    room.h
    imageviewer.h
    outlinepainter.h
    lib/glview.h
    lib/gldrawbuffersurface.h
    lib/glmaskedsurface.h
)

set(RESOURCE
    3dexplorer.qrc
)

project(${PROJECTNAME})

find_package(Qt53D REQUIRED)
find_package(Qt5Multimedia REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wunused-parameter")

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_AUTOMOC true)

qt5_add_resources(RESOURCE_RCC ${RESOURCE})

add_executable(${PROJECTNAME}
    ${SOURCE}
    ${HEADER_MOC}
    ${RESOURCE_RCC}
)

qt5_use_modules(${PROJECTNAME} 3D Multimedia)

target_link_libraries(${PROJECTNAME} GL)
