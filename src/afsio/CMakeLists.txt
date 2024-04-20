project(afsio)

add_library(${PROJECT_NAME} SHARED
	"afsio.cpp" "afsio.h" "afsio_addr.h" "afsio.rc"
	"afsreader.cpp" "afsreader.h"
)
register_module(${PROJECT_NAME})

# Include afsio directories for modules
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# AFSIO: dependencies
target_link_libraries(${PROJECT_NAME} pngdib)