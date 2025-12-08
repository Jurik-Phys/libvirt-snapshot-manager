add_executable(osinfo-loader main.cpp)

find_package(Qt6 REQUIRED COMPONENTS Network Xml)

target_link_libraries(osinfo-loader PRIVATE osinfoloader-lib
                                            Qt6::Core
                                            Qt6::Network
                                            Qt6::Xml
                                            liblzma_decoder)
