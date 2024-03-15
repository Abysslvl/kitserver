project(speeder)

add_library(${PROJECT_NAME} SHARED 
	speeder.cpp speeder.h speeder_addr.h speeder.rc
)

register_module(${PROJECT_NAME})