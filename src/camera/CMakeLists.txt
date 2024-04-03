project(camera)

add_library(${PROJECT_NAME} SHARED camera.cpp camera.h camera_addr.h camera.rc)

register_module(${PROJECT_NAME})