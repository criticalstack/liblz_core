add_library(lz_kvmap ${LIBLZ_OBJ_TYPE} lz_kvmap.c)

install(TARGETS lz_kvmap DESTINATION lib)
install(FILES lz_kvmap.h DESTINATION include/liblz)
