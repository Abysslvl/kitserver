project (kserv)

add_library(${PROJECT_NAME} SHARED
	"kserv.cpp" "kserv.h" "kserv_addr.h" "kserv.rc" 
	"teaminfo.h"
	${CONFIGS_OBJ}
	${GDB_OBJ}
)
register_module(${PROJECT_NAME})

# Kit Server: dependencies
target_link_libraries(${PROJECT_NAME} afsio)
target_link_libraries(${PROJECT_NAME} ZLIB)
target_link_libraries(${PROJECT_NAME} D3DX9)