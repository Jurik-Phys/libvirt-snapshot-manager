cmake_minimum_required(VERSION 3.20)

project(libvirt-snapshot-manager)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDART_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_BUILD_TYPE Debug)

option(BUILD_OSINFO_LOADER "Build osinfo loader utility" ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Xml)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/tools/shared)

qt_add_resources(RESOURCES ${CMAKE_SOURCE_DIR}/res/resources.qrc)

set(SOURCE_EXE main.cpp)

add_subdirectory(src)
add_subdirectory(tools/shared)
if (BUILD_OSINFO_LOADER)
    add_subdirectory(tools/osinfo-loader)
endif()

add_executable(libvirt-snapshot-manager ${SOURCE_EXE} ${RESOURCES})

target_link_libraries(libvirt-snapshot-manager PRIVATE src-lib Qt6::Widgets Qt6::Xml)
