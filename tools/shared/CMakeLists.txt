add_library(osinfoloader-lib STATIC osinfoloader.cpp)

target_link_libraries(osinfoloader-lib PRIVATE Qt6::Core)
