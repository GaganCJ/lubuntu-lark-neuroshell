cmake_minimum_required(VERSION 3.16)
project(LubuntuLarkNeuroshell LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Append -fno-rtti for safe WebKit/Ultralight linking
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus Network)
find_package(lxqt REQUIRED)
find_package(Ultralight REQUIRED)

# 1. AI Widget Shared Library (ILXQtPanelPlugin component)
add_library(plugin-neuroshell SHARED aiwidget.cpp aiwidget.h)
target_link_libraries(plugin-neuroshell PRIVATE Qt6::Widgets Qt6::DBus lxqt Ultralight::Ultralight)

# 2. Config Window Executable
add_executable(lxqt-config-neuroshell modelmanager.cpp modelmanager.h)
target_link_libraries(lxqt-config-neuroshell PRIVATE Qt6::Widgets Qt6::Network lxqt Ultralight::Ultralight)

# 3. MemFusion Config Window Executable
add_executable(lxqt-config-memfusion memfusionconfig.cpp memfusionconfig.h)
target_link_libraries(lxqt-config-memfusion PRIVATE Qt6::Widgets lxqt Ultralight::Ultralight)

install(TARGETS lxqt-config-neuroshell DESTINATION bin)
install(TARGETS lxqt-config-memfusion DESTINATION bin)