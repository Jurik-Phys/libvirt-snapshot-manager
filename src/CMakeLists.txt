cmake_minimum_required(VERSION 3.20)

set(SOURCE_LIB  appInfo.cpp
                appWindow.cpp
                snapManager.cpp
                snapTreeModel.cpp
                snapTreeView.cpp
                vmDataCollector.cpp
                treeItemDelegate.cpp
                treeLinesStyle.cpp
                vmInfoWidget.cpp
                snapInfoWidget.cpp
                vmWidget.cpp
                osInfoProvider.cpp
                nodeInfoProvider.cpp
                dialogAboutApp.cpp
                dialogAboutThirdParty.cpp
                dialogAddNewImage.cpp
                dialogDeleteImage.cpp
                dialogManageCpu.cpp
                dialogManageGuestOS.cpp
                dialogManageRam.cpp)

add_library(src-lib STATIC ${SOURCE_LIB})

target_link_libraries(src-lib PRIVATE Qt6::Widgets
                                      Qt6::Network
                                      Qt6::SvgWidgets
                                      Qt6::Xml
                                      osinfoloader-lib
                                      liblzma_decoder)
