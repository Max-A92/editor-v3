cmake_minimum_required(VERSION 3.16)
project(editor_v5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport Charts Sql)

add_executable(editor_v5
    main.cpp
    mainwindow.cpp
    mainwindow.h
    richtexteditor.cpp
    richtexteditor.h
    specialchardialog.cpp
    specialchardialog.h
    tabledialog.cpp
    tabledialog.h
    ngramanalyzer.cpp
    ngramanalyzer.h
    ngramtablemodel.cpp
    ngramtablemodel.h
    calculatordialog.cpp
    calculatordialog.h
    databasemanager.cpp
    databasemanager.h
    bugreportdialog.cpp
    bugreportdialog.h
)

set_target_properties(editor_v5 PROPERTIES WIN32_EXECUTABLE ON)

target_link_libraries(editor_v5
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
    Qt6::Charts
    Qt6::Sql
)
