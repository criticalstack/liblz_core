macro (add_sources)
	file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")

	foreach (_src ${ARGN})
		if (_relPath)
			list (APPEND SRCS "${_relPath}/${_src}")
		else()
			list (APPEND SRCS "${_src}")
		endif()
	endforeach()

	if (_relPath)
		set (SRCS ${SRCS} PARENT_SCOPE)
	endif()
endmacro()

add_subdirectory (heap)
add_subdirectory (kvmap)
add_subdirectory (tailq)

install (FILES   liblz.h DESTINATION include)
install (FILES   lzapi.h DESTINATION include/liblz)

set (LZ_SOURCES ${SRCS} PARENT_SCOPE)
