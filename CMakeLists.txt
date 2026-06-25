cmake_minimum_required(VERSION 3.16)
project(lubuntu-lark-neuroshell LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(lxqt REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets WebEngineWidgets REQUIRED)

add_library(lxqt_ai_widget MODULE aiwidget.cpp aiwidget.h)
target_link_libraries(lxqt_ai_widget 
    Qt6::Widgets 
    Qt6::WebEngineWidgets 
    lxqt
)

install(TARGETS lxqt_ai_widget DESTINATION "${LXQT_PANEL_PLUGINS_DIR}")