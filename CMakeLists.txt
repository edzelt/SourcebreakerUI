cmake_minimum_required(VERSION 3.16)

project(SourcebreakerUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Без .ui файлов — AUTOUIC не нужен
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

list(APPEND CMAKE_PREFIX_PATH
    "C:/Qt/6.10.2/msvc2022_64"
    "D:/Libs/KDDockWidgets"
    "C:/Program Files/LLVM"
)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)
find_package(KDDockWidgets-qt6 REQUIRED)

find_path(CLANG_INCLUDE_DIR clang-c/Index.h
    PATHS "C:/Program Files/LLVM/include")
find_library(CLANG_LIBRARY NAMES libclang
    PATHS "C:/Program Files/LLVM/lib")

add_executable(SourcebreakerUI
    main.cpp
    MainWindow.h
    MainWindow.cpp
    Storage.h
    Storage.cpp
)

target_link_libraries(SourcebreakerUI PRIVATE
    Qt6::Widgets
    Qt6::Sql
    KDAB::kddockwidgets
    ${CLANG_LIBRARY}
)

qt_finalize_executable(SourcebreakerUI)