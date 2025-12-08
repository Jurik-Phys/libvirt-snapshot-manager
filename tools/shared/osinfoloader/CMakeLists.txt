
find_package(Qt6 REQUIRED COMPONENTS Network Xml)

add_library(osinfoloader-lib STATIC osinfoloader.cpp)

target_link_libraries(osinfoloader-lib PRIVATE Qt6::Core
                                               Qt6::Network
                                               Qt6::Xml)
