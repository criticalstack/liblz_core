add_library(lz_tailq ${LIBLZ_OBJ_TYPE} lz_tailq.c)
target_link_libraries(lz_tailq lz_heap)

install(TARGETS lz_tailq DESTINATION lib)
install(FILES lz_tailq.h DESTINATION include/liblz)
