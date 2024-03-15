project(pngdib)

add_library(${PROJECT_NAME} STATIC pngdib.c pngdib.h)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} ZLIB)
target_link_libraries(${PROJECT_NAME} PNG)