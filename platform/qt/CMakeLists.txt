cmake_minimum_required(VERSION 3.13)
project(QtEditor)
set(CMAKE_PREFIX_PATH "F:/Qt/5.8/mingw53_32/lib/cmake")

#打开Qt的MOC功能和UI文件处理功能
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(USE_QT Core Gui Widgets)
find_package(Qt5 COMPONENTS ${USE_QT} REQUIRED)
include_directories(.)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} qt_editor_sources)
add_executable(qt_editor ${qt_editor_sources} mainwindow.ui)
qt5_use_modules(qt_editor ${USE_QT})
target_link_libraries(qt_editor ${LINK_STATIC_LIB})
