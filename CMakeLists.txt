cmake_minimum_required(VERSION 3.16)
project(commitly VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/titlebar.cpp
    src/sidebar.cpp
    src/changestab.cpp
    src/historytab.cpp
    src/diffviewer.cpp
    src/commitdetailview.cpp
    src/mockdata.cpp
    src/syncheader.cpp
)

set(HEADERS
    src/mainwindow.h
    src/titlebar.h
    src/sidebar.h
    src/changestab.h
    src/historytab.h
    src/diffviewer.h
    src/commitdetailview.h
    src/mockdata.h
    src/models.h
    src/syncheader.h
)

add_executable(commitly ${SOURCES} ${HEADERS})

target_link_libraries(commitly PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)

target_include_directories(commitly PRIVATE src)