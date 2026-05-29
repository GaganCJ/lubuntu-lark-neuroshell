# CMakeLists.txt for Lubuntu Lark Ecosystem

cmake_minimum_required(VERSION 3.16)
project(LubuntuLark LANGUAGES CXX)

# --- Standard Settings ---
# Use C++17 and enable Qt's automatic tools
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# --- Find Dependencies ---

# 1. Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus Network)

# 2. Find LXQt Panel development files
# This provides the LXQt::PanelPlugin target for linking panel plugins
find_package(lxqt-panel REQUIRED)


# --- Define Project Targets ---

# 1. NeuroShell Panel Plugin
add_library(neuroshell SHARED aiwidget.cpp)
target_link_libraries(neuroshell PRIVATE LXQt::PanelPlugin Qt6::DBus Qt6::Widgets)
set_target_properties(neuroshell PROPERTIES OUTPUT_NAME "neuroshell")

# 2. NeuroShell Config App
add_executable(lxqt-config-neuroshell modelmanager.cpp)
target_link_libraries(lxqt-config-neuroshell PRIVATE Qt6::Widgets Qt6::Network)

# 3. MemFusion Config App
add_executable(lxqt-config-memfusion memfusionconfig.cpp)
target_link_libraries(lxqt-config-memfusion PRIVATE Qt6::Widgets)


# --- Installation Rules ---
include(GNUInstallDirs)

install(TARGETS neuroshell LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/lxqt-panel)
install(TARGETS lxqt-config-neuroshell lxqt-config-memfusion RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

message(STATUS "Configuration complete. Run 'make' and 'sudo make install'.")