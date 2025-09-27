cmake_minimum_required(VERSION 3.20)
project(WinShellPreview VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows特有の設定
if(NOT WIN32)
    message(FATAL_ERROR "This project is Windows only")
endif()

# Unicode設定
add_definitions(-DUNICODE -D_UNICODE)

# 出力ディレクトリ設定
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# サブディレクトリを追加
add_subdirectory(WinShellPreview)
add_subdirectory(TestApp)