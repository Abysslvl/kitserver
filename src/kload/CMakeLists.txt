project(kload)

# kitserver global configure file
configure_file(kconf.h.in ${CONFIGURE_DIR}/kconf.h)

add_library(${PROJECT_NAME} SHARED
	kload.cpp kload.h kload_addr.h kload.rc
	hook.cpp hook.h hook_addr.h
	hooklib.cpp hooklib.h
	crc32.cpp crc32.h
	log.cpp log.h
	${CONFIGS_OBJ}
	${IMAGEUTIL_OBJ}
	${DETECT_OBJ}
	${LANG_OBJ}
)

# include kitserver & kload directories for modules
target_include_directories(${PROJECT_NAME} PUBLIC ${SHARED_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${CONFIGURE_DIR})
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# set global compile options & definitions for modules
target_compile_options(${PROJECT_NAME} PUBLIC ${KITSERVER_COMPILE_OPTIONS})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${KITSERVER_COMPILE_DEFINITIONS})

# for link modules to kload & afsio, and set installations
function(register_module MODULE_TARGET)
	if(NOT ${MODULE_TARGET} STREQUAL "kload")
		target_link_libraries(${MODULE_TARGET} kload)
		if(NOT ${MODULE_TARGET} STREQUAL "afsio")
			target_link_libraries(${MODULE_TARGET} afsio)
		endif()
	endif()
	install(TARGETS ${MODULE_TARGET} 
	RUNTIME DESTINATION ${INSTALL_DIR})
	message("${MODULE_TARGET} module is registered.")
endfunction()

register_module(${PROJECT_NAME})

# necessary to kload & manager works
install(FILES ${SHARED_DIR}/lang_eng.txt DESTINATION ${INSTALL_DIR})
