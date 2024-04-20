project (afs2fs)

add_library(${PROJECT_NAME} SHARED 
	"afs2fs.cpp" "afs2fs.h" "afs2fs_addr.h" "afs2fs.rc"
	${NAMES_OBJ}
	${CONFIGS_OBJ} 
)
register_module(${PROJECT_NAME})

# AFS2FS: dependencies
target_link_libraries(${PROJECT_NAME} shlwapi.lib)
target_link_libraries(${PROJECT_NAME} afsio)