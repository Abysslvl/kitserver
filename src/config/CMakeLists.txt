project(config)

add_executable(${PROJECT_NAME} WIN32
	lodcfg.cpp lodcfg.h
	lodcfgui.cpp lodcfgui.h
	config.rc
	${CONFIGS_OBJ}
	${VISUAL_STYLES}
)

# Set global compile options & definitions
target_compile_options(${PROJECT_NAME} PRIVATE ${KITSERVER_COMPILE_OPTIONS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${KITSERVER_COMPILE_DEFINITIONS})

# Include kitserver & lodmixer directories
target_include_directories(${PROJECT_NAME} PRIVATE ${SHARED_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CONFIGURE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lodmixer)

# Link to config dependencies
target_link_libraries(${PROJECT_NAME} comctl32.lib)

# For the installation
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_DIR})
