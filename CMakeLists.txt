cmake_minimum_required(VERSION 3.20)

project(libvirt-snapshot-manager VERSION 0.3.1)

# *** Application version info *** #
#         ${APP_VERSION}           #
#         ${APP_GIT_HASH}          #
#         ${APP_GIT_BRANCH}        #
#         ${APP_GIT_DIRTY}         #
# ******************************** #
add_compile_definitions(
    APP_VERSION="${PROJECT_VERSION}"
)

execute_process(
    COMMAND git rev-parse --short HEAD
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git status --porcelain
    OUTPUT_VARIABLE GIT_STATUS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(GIT_DIRTY "")
if(GIT_STATUS)
    set(GIT_DIRTY "-dirty")
endif()

add_compile_definitions(
    APP_GIT_HASH="${GIT_HASH}"
    APP_GIT_BRANCH="${GIT_BRANCH}"
    APP_GIT_DIRTY="${GIT_DIRTY}"
)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDART_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_BUILD_TYPE Debug)

option(BUILD_OSINFO_LOADER "Build osinfo loader utility" ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Xml SvgWidgets)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/tools/shared/osinfoloader)
include_directories(
    ${CMAKE_SOURCE_DIR}/tools/shared/thirdparty/xz_decompress/src/common
    ${CMAKE_SOURCE_DIR}/tools/shared/thirdparty/xz_decompress/src/liblzma/api
    ${CMAKE_SOURCE_DIR}/tools/shared/thirdparty/xz_decompress/src/liblzma/common
    ${CMAKE_SOURCE_DIR}/tools/shared/thirdparty/xz_decompress/src/liblzma/lzma
    ${CMAKE_SOURCE_DIR}/tools/shared/thirdparty/xz_decompress/src/liblzma/lz
    ${CMAKE_SOURCE_DIR}/tools/shared/thirdparty/xz_decompress/src/liblzma/delta
    ${CMAKE_SOURCE_DIR}/tools/shared/thirdparty/xz_decompress/src/liblzma/rangecoder
    ${CMAKE_SOURCE_DIR}/tools/shared/thirdparty/xz_decompress/src/liblzma/simple
    ${CMAKE_SOURCE_DIR}/tools/shared/thirdparty/xz_decompress/src/liblzma/container
    ${CMAKE_SOURCE_DIR}/tools/shared/thirdparty/xz_decompress/src/liblzma/check
    ${CMAKE_SOURCE_DIR}/tools/shared/thirdparty/xz_decompress/src/liblzma/tuklib
)

qt_add_resources(RESOURCES ${CMAKE_SOURCE_DIR}/res/resources.qrc)

set(SOURCE_EXE main.cpp)

add_subdirectory(src)
add_subdirectory(tools/shared/osinfoloader)
if (BUILD_OSINFO_LOADER)
    add_subdirectory(tools/osinfo-loader)
endif()
add_subdirectory(tools/shared/thirdparty/xz_decompress)

add_executable(libvirt-snapshot-manager ${SOURCE_EXE} ${RESOURCES})

target_link_libraries(libvirt-snapshot-manager PRIVATE src-lib
                                                       Qt6::Widgets
                                                       Qt6::Network
                                                       Qt6::Xml)
