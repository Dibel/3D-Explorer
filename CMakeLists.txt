cmake_minimum_required(VERSION 2.8.11)

set(PROJECTNAME 3dexplorer)

set(SOURCE
    main.cpp
    pickobject.cpp
    meshobject.cpp
    imageobject.cpp
    room.cpp
    view.cpp
    paint.cpp
    control.cpp
    directory.cpp
    animation.cpp
    config.cpp
    lib/glview.cpp
    lib/qgldrawbuffersurface.cpp
    lib/qglmaskedsurface.cpp
)

set(HEADER
    common.h
    pickobject.h
    meshobject.h
    imageobject.h
    room.h
    view.h
    directory.h
    lib/glview.h
    lib/qgldrawbuffersurface_p.h
    lib/qglmaskedsurface_p.h
)

set(RESOURCE
    3dexplorer.qrc
)

project(${PROJECTNAME})

find_package(Qt53D REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wunused-parameter")

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_AUTOMOC true)

qt5_add_resources(RESOURCE_RCC ${RESOURCE})

add_executable(${PROJECTNAME}
    ${SOURCE}
    ${HEADER_MOC}
    ${RESOURCE_RCC}
)

qt5_use_modules(${PROJECTNAME} 3D)

target_link_libraries(${PROJECTNAME} GL)
