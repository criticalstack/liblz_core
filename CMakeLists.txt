cmake_minimum_required (VERSION 2.8)
project (liblz_core)
include (CheckCCompilerFlag)
include (CheckIncludeFiles)

set    (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
option (ENABLE_STATIC "Enable Static Libraries" Off)
option (ENABLE_SHARED "Enable Shared Libraries [DEFAULT]" On)

if (ENABLE_STATIC)
	unset (ENABLE_SHARED)
	set   (LIBLZ_OBJ_TYPE STATIC)
else()
	set   (LIBLZ_OBJ_TYPE SHARED)
endif()

set (LIBLZ_INS_PATH ${PROJECT_BINARY_DIR}/install)
set (LIBLZ_LIB_PATH ${LIBLZ_INS_PATH}/lib)
set (LIBLZ_INC_PATH ${LIBLZ_INS_PATH}/include)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBLZ_LIB_PATH})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBLZ_LIB_PATH})

include_directories (${LIBLZ_INC_PATH}
                     ${CMAKE_CURRENT_SOURCE_DIR}/src)

check_include_files   (sys/tree.h          HAS_SYS_TREE)
check_include_files   (sys/queue.h         HAS_SYS_QUEUE)
check_c_compiler_flag (-fvisibility=hidden HAS_VISIBILITY_HIDDEN)

if (HAS_VISIBILITY_HIDDEN)
	set             (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
	add_definitions (-DHAS_VISIBILITY_HIDDEN)
endif ()

# create a copy of sys/tree.h if not found
if (NOT HAS_SYS_TREE)
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/compat/sys/tree.h.in
	                ${PROJECT_BINARY_DIR}/include/liblz/sys/tree.h)

endif ()

# create a copy of sys/queue.h if not found
if (NOT HAS_SYS_QUEUE)
	configure_file (${CMAKE_CURRENT_SOURCE_DIR}/compat/sys/queue.h.in
	                ${PROJECT_BINARY_DIR}/include/liblz/sys/queue.h)
endif ()


add_subdirectory(src)
