project(leagues)

add_library(${PROJECT_NAME} SHARED
	"leagues.cpp" "leagues.h" "leagues_addr.h" "leagues.rc"
)

register_module(${PROJECT_NAME})