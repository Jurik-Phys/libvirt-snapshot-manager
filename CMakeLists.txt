cmake_minimum_required(VERSION 3.20)

project(libvirt-ext-snap-gui)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDART_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Xml)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(SOURCE_EXE main.cpp)

add_subdirectory(src)

add_executable(libvirt-snapshots-manager ${SOURCE_EXE})

target_link_libraries(libvirt-snapshots-manager PRIVATE src-lib Qt6::Widgets Qt6::Xml)
