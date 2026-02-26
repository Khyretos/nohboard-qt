cmake_minimum_required(VERSION 3.16)
project(NohBoard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)

# Platform detection options (default ON)
option(BUILD_X11 "Build with X11 support" ON)
option(BUILD_WAYLAND "Build with Wayland support" ON)

# Core sources (always included)
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/keyboardwidget.cpp
    src/logger.cpp
    src/input/inputhook_factory.cpp
    src/input/inputmanager.cpp
    src/loaders/keyboardloader.cpp
    src/loaders/styleloader.cpp
    src/loaders/definitionloader.cpp
    src/dialogs/styledialog.cpp
    src/dialogs/settingsdialog.cpp
    src/dialogs/loaddialog.cpp
    src/dialogs/keyboardpropertiesdialog.cpp
    src/dialogs/elementpropertiesdialog.cpp
    src/dialogs/keyboardstyledialog.cpp
    src/dialogs/keystyledialog.cpp
    keyboards.qrc
)

# Headers
set(HEADERS
    src/mainwindow.h
    src/keyboardwidget.h
    src/input/inputhook.h
)

# Platform-specific input hooks
set(HAS_X11 FALSE)
set(HAS_WAYLAND FALSE)

if(WIN32)
    list(APPEND SOURCES src/input/inputhook_win.cpp)
    add_compile_definitions(USE_WINDOWS)
elseif(APPLE)
    list(APPEND SOURCES src/input/inputhook_mac.cpp)
    add_compile_definitions(USE_MACOS)
elseif(UNIX AND NOT APPLE)
    # Linux – check for X11 and Wayland
    if(BUILD_X11)
        find_package(X11)
        if(X11_FOUND)
            if(NOT X11_Xrecord_FOUND)
                find_library(X11_Xrecord_LIB Xtst)
                if(X11_Xrecord_LIB)
                    set(X11_Xrecord_FOUND TRUE)
                endif()
            endif()
            if(X11_Xrecord_FOUND)
                set(HAS_X11 TRUE)
                list(APPEND SOURCES src/input/inputhook_x11.cpp)
                add_compile_definitions(USE_X11)
                message(STATUS "✓ X11 support enabled")
            else()
                message(STATUS "✗ X11 Record extension not found – X11 disabled")
            endif()
        else()
            message(STATUS "✗ X11 not found – disabled")
        endif()
    endif()

    if(BUILD_WAYLAND)
        find_package(PkgConfig)
        if(PkgConfig_FOUND)
            pkg_check_modules(LIBINPUT libinput)
            pkg_check_modules(LIBUDEV libudev)
            if(LIBINPUT_FOUND AND LIBUDEV_FOUND)
                set(HAS_WAYLAND TRUE)
                list(APPEND SOURCES src/input/inputhook_wayland.cpp)
                add_compile_definitions(USE_WAYLAND)
                message(STATUS "✓ Wayland support enabled")
            else()
                message(STATUS "✗ Wayland dependencies (libinput/libudev) not found – disabled")
            endif()
        else()
            message(STATUS "✗ pkg-config not found – Wayland disabled")
        endif()
    endif()

    if(NOT HAS_X11 AND NOT HAS_WAYLAND)
        message(WARNING "No Linux input hook built – keyboard capture may not work!")
    endif()
endif()

# Create executable
add_executable(NohBoard ${SOURCES} ${HEADERS})

# Link Qt libraries
target_link_libraries(NohBoard PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
)

# Link platform-specific libraries
if(HAS_X11)
    target_link_libraries(NohBoard PRIVATE
        ${X11_LIBRARIES}
        ${X11_Xrecord_LIB}
    )
    target_include_directories(NohBoard PRIVATE ${X11_INCLUDE_DIR})
endif()

if(HAS_WAYLAND)
    target_link_libraries(NohBoard PRIVATE
        ${LIBINPUT_LIBRARIES}
        ${LIBUDEV_LIBRARIES}
    )
    target_include_directories(NohBoard PRIVATE
        ${LIBINPUT_INCLUDE_DIRS}
        ${LIBUDEV_INCLUDE_DIRS}
    )
endif()

# Include directories
target_include_directories(NohBoard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input
)

# Installation (basic – install the executable)
install(TARGETS NohBoard RUNTIME DESTINATION bin)

# Windows specific settings
if(WIN32)
    # Force static linking of Qt (for Windows)
    set(Qt6_STATIC ON)

    # Statically link MinGW runtime and make it a GUI application
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    target_link_options(NohBoard PRIVATE -mwindows)
    set_target_properties(NohBoard PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE FALSE
    )
    # Add icon resource (requires nohboard.ico in project root)
    target_sources(NohBoard PRIVATE resources.rc)
endif()

# Mac specific settings
if(APPLE)
    set_target_properties(NohBoard PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourname.NohBoard"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
    )
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "═══════════════════════════════════════════")
message(STATUS "  NohBoard Configuration Summary")
message(STATUS "═══════════════════════════════════════════")
if(HAS_X11)
    message(STATUS "  ✓ X11 support:        ENABLED")
else()
    message(STATUS "  ✗ X11 support:        DISABLED")
endif()
if(HAS_WAYLAND)
    message(STATUS "  ✓ Wayland support:    ENABLED")
else()
    message(STATUS "  ✗ Wayland support:    DISABLED")
endif()
message(STATUS "  Qt6 version:          ${Qt6_VERSION}")
message(STATUS "  Build type:           ${CMAKE_BUILD_TYPE}")
message(STATUS "═══════════════════════════════════════════")
message(STATUS "")