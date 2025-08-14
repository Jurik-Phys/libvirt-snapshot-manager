cmake_minimum_required(VERSION 3.20)

project(manager-for-external-snapshots-for-libvirt)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDART_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(Qt6 REQUIRED COMPONENTS Widgets Xml)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(SOURCE_EXE main.cpp)

add_subdirectory(src)

add_executable(libvirt-snapshot-manager ${SOURCE_EXE})

target_link_libraries(libvirt-snapshot-manager PRIVATE src-lib Qt6::Widgets Qt6::Xml)
