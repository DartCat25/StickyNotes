cmake_minimum_required(VERSION 3.16)
project(StickyNotes VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Quick)

# Define your source files
set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/MainWindow.ui
)

add_executable(StickyNotes ${PROJECT_SOURCES})

target_include_directories(StickyNotes PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(StickyNotes PRIVATE 
Qt6::Core 
Qt6::Gui 
Qt6::Widgets 
Qt6::Quick
)

