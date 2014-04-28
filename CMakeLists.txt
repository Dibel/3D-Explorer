cmake_minimum_required(VERSION 2.8.11)

set(PROJECTNAME 3dexplorer)

set(SOURCE
    main.cpp
    view.cpp
    meshobject.cpp
)

set(HEADER
    view.h
    meshobject.h
)

set(RESOURCE
    3dexplorer.qrc
)

project(${PROJECTNAME})

find_package(Qt53D REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_AUTOMOC true)

qt5_add_resources(RESOURCE_RCC ${RESOURCE})

add_executable(${PROJECTNAME}
    ${SOURCE}
    ${HEADER_MOC}
    ${RESOURCE_RCC}
)

qt5_use_modules(${PROJECTNAME} 3D OpenGL Widgets)

target_link_libraries(${PROJECTNAME} GL)
