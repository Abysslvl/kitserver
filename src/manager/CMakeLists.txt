project(manager)

add_executable(${PROJECT_NAME} WIN32
	manager.cpp manager.h manager.rc
	setupgui.cpp setupgui.h
	${CONFIGS_OBJ}
	${IMAGEUTIL_OBJ}
	${DETECT_OBJ}
	${LANG_OBJ}
)

# set global compile options & definitions
target_compile_options(${PROJECT_NAME} PRIVATE ${KITSERVER_COMPILE_OPTIONS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${KITSERVER_COMPILE_DEFINITIONS})

# include kitserver directories
target_include_directories(${PROJECT_NAME} PRIVATE ${SHARED_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CONFIGURE_DIR})

# for the installation
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_DIR})