project(sides)

add_library(${PROJECT_NAME} SHARED
	"sides.cpp" "sides.h" "sides_addr.h" "sides.rc"
)
register_module(${PROJECT_NAME})