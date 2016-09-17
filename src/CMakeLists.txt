add_subdirectory(heap)
add_subdirectory(kvmap)
add_subdirectory(tailq)

install(FILES liblz.h DESTINATION include/)
install(FILES lzapi.h DESTINATION include/liblz/)
