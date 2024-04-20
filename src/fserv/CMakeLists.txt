project(fserv)

add_library(${PROJECT_NAME} SHARED
	"fserv.cpp" "fserv.h" "fserv_addr.h" "fserv.rc" 
	"replay.h"
)
register_module(${PROJECT_NAME})

# Face Server: dependencies
target_link_libraries(${PROJECT_NAME} afsio)