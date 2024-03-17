cmake_minimum_required (VERSION 3.28.0)
project (kitserver VERSION 13.0.0.0)

set(KITSERVER_COPYRIGHT "Copyright (c) 2005-2012 Juce, Robbie, jminhano")

# Thirdparty dependencies
find_library(ZLIB_LIB zdll PATHS ${CMAKE_SOURCE_DIR}/thirdparty/zlib-1.2.3/lib)
if(ZLIB_LIB)
    add_library(ZLIB SHARED IMPORTED)
    message("ZLIB is found!.")

    # Set zlib target
    set(ZLIB_DIR ${CMAKE_SOURCE_DIR}/thirdparty/zlib-1.2.3)
	
    set_target_properties(ZLIB PROPERTIES IMPORTED_IMPLIB ${ZLIB_LIB})
    target_include_directories(ZLIB INTERFACE ${ZLIB_DIR}/include)	
else()
    message("ZLIB not found. Please install it.")
endif()

find_library(PNG_LIB libpng PATHS ${CMAKE_SOURCE_DIR}/thirdparty/libpng-1.2.8/lib)
if(PNG_LIB)
    add_library(PNG SHARED IMPORTED)
    message("LIBPNG is found!.")

    # Set libpng target
    set(PNG_DIR ${CMAKE_SOURCE_DIR}/thirdparty/libpng-1.2.8)
 
    set_target_properties(PNG PROPERTIES IMPORTED_IMPLIB ${PNG_LIB})
    target_include_directories(PNG INTERFACE ${PNG_DIR}/include)
else()
    message("LIBPNG not found. Please install it.")
endif()

find_library(D3DX9_LIB d3dx9 PATHS ${CMAKE_SOURCE_DIR}/thirdparty/dxsdk/lib/x86)
if(D3DX9_LIB)
    add_library(D3DX9 SHARED IMPORTED)
    message("D3DX9 is found!.")
    
    # Set D3DX9 target
    set(DXSDK_DIR ${CMAKE_SOURCE_DIR}/thirdparty/dxsdk)
    
    set_target_properties(D3DX9 PROPERTIES IMPORTED_IMPLIB ${D3DX9_LIB})
    target_include_directories(D3DX9 INTERFACE ${DXSDK_DIR}/include)
else()
    message("D3DX9 not found. Please install it.")
endif()

add_subdirectory(thirdparty/pngdib)

# MSVC compile settings
option(KITSERVER_RELEASE_BUILD "Release log type." ON)

set(KITSERVER_COMPILE_OPTIONS
    /Od         # Disable optimizations during compilation
    /EHsc       # Enables standard c++ exception handling
    /wd4731     # Surpresses the 4731 compiler warning
)

set(KITSERVER_COMPILE_DEFINITIONS
    _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS    # Suppress deprecation warnings of stdext::hash
)

if(KITSERVER_RELEASE_BUILD)
    list(APPEND KITSERVER_COMPILE_DEFINITIONS MYDLL_RELEASE_BUILD)
else()
    list(APPEND KITSERVER_COMPILE_DEFINITIONS DEBUG)
endif()

message("Kitserver compile options: ${KITSERVER_COMPILE_OPTIONS}")
message("Kitserver compile definitions: ${KITSERVER_COMPILE_DEFINITIONS}")

# Kitserver directories
set(SHARED_DIR ${CMAKE_SOURCE_DIR}/src/shared)
set(CONFIGURE_DIR ${CMAKE_BINARY_DIR}/src/kconf)
set(INSTALL_DIR ${CMAKE_SOURCE_DIR}/install)
set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)

message("Kitserver directories:
SHARED_DIR: ${SHARED_DIR}
CONFIGURE_DIR: ${CONFIGURE_DIR}
INSTALL_DIR: ${INSTALL_DIR}
THIRDPARTY_DIR: ${THIRDPARTY_DIR}"
)

# Kitserver shared objetcs
set(CONFIGS_OBJ 
    ${SHARED_DIR}/configs.cpp ${SHARED_DIR}/configs.h ${SHARED_DIR}/configs.hpp
    ${SHARED_DIR}/utf8.h
)

set(NAMES_OBJ
    ${SHARED_DIR}/names.cpp ${SHARED_DIR}/names.h
)

set(GDB_OBJ
    ${SHARED_DIR}/gdb.cpp ${SHARED_DIR}/gdb.h
    ${SHARED_DIR}/configs.hpp
)

set(DETECT_OBJ
    ${SHARED_DIR}/detect.cpp ${SHARED_DIR}/detect.h
)

set(LANG_OBJ
    ${SHARED_DIR}/lang.cpp ${SHARED_DIR}/lang.h
)

set(IMAGEUTIL_OBJ
    ${SHARED_DIR}/imageutil.cpp ${SHARED_DIR}/imageutil.h
)

# Kitserver modules
add_subdirectory(src/kload)
add_subdirectory(src/manager)
add_subdirectory(src/config)
add_subdirectory(src/afsio)
add_subdirectory(src/afs2fs)
#add_subdirectory(src/kserv)
#add_subdirectory(src/fserv)
add_subdirectory(src/lodmixer)
add_subdirectory(src/speeder)
#add_subdirectory(src/sides)

# Install dependencies
install(
    FILES ${THIRDPARTY_DIR}/zlib-1.2.3/bin/zlib1.dll ${THIRDPARTY_DIR}/libpng-1.2.8/bin/libpng13.dll
    DESTINATION ${INSTALL_DIR}
)