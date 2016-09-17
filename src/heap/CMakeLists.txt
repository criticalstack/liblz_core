add_library(lz_heap ${LIBLZ_OBJ_TYPE} lz_heap.c)

install(TARGETS lz_heap DESTINATION lib)
install(FILES lz_heap.h DESTINATION include/liblz)
