add_executable(osinfo-loader main.cpp)

target_link_libraries(osinfo-loader PRIVATE osinfoloader-lib Qt6::Core)
