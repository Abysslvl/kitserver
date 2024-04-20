project(lodmixer)

add_library(${PROJECT_NAME} SHARED
	"lodmixer.cpp" "lodmixer.h" "lodmixer_addr.h" "lodmixer.rc"
)
register_module(${PROJECT_NAME})