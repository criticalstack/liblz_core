
add_library (lz_core
			 heap.c
			 kvmap.c
			 tailq.c
			 ffile.c
)

install (TARGETS lz_core DESTINATION lib)

install (FILES liblz.h
		 DESTINATION include)

install (FILES lzapi.h
		 DESTINATION include/liblz)

install (FILES heap.h
         DESTINATION include/liblz/core
         RENAME      lz_heap.h)

install (FILES kvmap.h
         DESTINATION include/liblz/core
         RENAME      lz_kvmap.h
)

install (FILES tailq.h
         DESTINATION include/liblz/core
         RENAME      lz_tailq.h
)

install (FILES ffile.h
		 DESTINATION include/liblz/core
		 RENAME      lz_file.h
)

configure_file (${CMAKE_SOURCE_DIR}/src/heap.h
                ${PROJECT_BINARY_DIR}/include/liblz/core/lz_heap.h)

configure_file (${CMAKE_SOURCE_DIR}/src/kvmap.h
                ${CMAKE_BINARY_DIR}/include/liblz/core/lz_kvmap.h)

configure_file (${CMAKE_SOURCE_DIR}/src/tailq.h
                ${CMAKE_BINARY_DIR}/include/liblz/core/lz_tailq.h)

configure_file (${CMAKE_SOURCE_DIR}/src/ffile.h
				${CMAKE_BINARY_DIR}/include/liblz/core/lz_file.h)

configure_file (${CMAKE_SOURCE_DIR}/src/lzapi.h
                ${CMAKE_BINARY_DIR}/include/liblz)

configure_file (${CMAKE_SOURCE_DIR}/src/liblz.h
                ${CMAKE_BINARY_DIR}/include)

