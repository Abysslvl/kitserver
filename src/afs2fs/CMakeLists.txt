project (afs2fs)

add_library(${PROJECT_NAME} SHARED 
	afs2fs.cpp afs2fs.h afs2fs_addr.h afs2fs.rc
	${NAMES_OBJ}
	${CONFIGS_OBJ} 
)

# Link to afs2fs dependencies
target_link_libraries(${PROJECT_NAME} shlwapi.lib)

register_module(${PROJECT_NAME})