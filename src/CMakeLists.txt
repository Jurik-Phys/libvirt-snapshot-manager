cmake_minimum_required(VERSION 3.20)

set(SOURCE_LIB  appWindow.cpp
                snapManager.cpp
                snapTreeModel.cpp
                vmDataCollector.cpp
                treeItemDelegate.cpp
                treeLinesStyle.cpp
                vmWidget.cpp)

add_library(src-lib STATIC ${SOURCE_LIB})

target_link_libraries(src-lib PRIVATE Qt6::Widgets Qt6::Xml)
