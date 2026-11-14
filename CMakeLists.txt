cmake_minimum_required(VERSION 3.16)
project(StickyNotes VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Widgets)

# Define your source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc")

# WIN32 makes the app a GUI application on Windows (no console window).
# The keyword is ignored on other platforms.
add_executable(StickyNotes WIN32 ${SOURCES})

target_include_directories(StickyNotes PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    lib
)

target_link_libraries(StickyNotes PRIVATE 
Qt6::Core 
Qt6::Gui 
Qt6::Widgets
)

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED COMPONENTS Xext)
    target_link_libraries(StickyNotes PRIVATE X11::X11)

    find_package(PkgConfig REQUIRED)
        pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb)

    target_link_libraries(StickyNotes PRIVATE
        Qt6::Widgets
        PkgConfig::XCB
        X11::Xext
    )
endif()
