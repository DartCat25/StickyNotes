cmake_minimum_required(VERSION 3.16)
project(StickyNotes VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Define your source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# WIN32 makes the app a GUI application on Windows (no console window).
# The keyword is ignored on other platforms.
add_executable(StickyNotes WIN32 ${SOURCES})

target_include_directories(StickyNotes PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(StickyNotes PRIVATE 
Qt6::Core 
Qt6::Gui 
Qt6::Widgets
)

# Static Qt does not find its platform plugin at runtime unless it is linked in.
get_target_property(QT6_CORE_TYPE Qt6::Core TYPE)
if(QT6_CORE_TYPE STREQUAL "STATIC_LIBRARY")
    if(WIN32)
        qt_import_plugins(StickyNotes INCLUDE Qt6::QWindowsIntegrationPlugin)
    endif()

    # Link the compiler runtime statically as well, so the exe is self-contained.
    if(MINGW)
        target_link_options(StickyNotes PRIVATE -static)
    elseif(MSVC)
        set_property(TARGET StickyNotes PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

if(LINUX)
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
