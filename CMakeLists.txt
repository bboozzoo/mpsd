cmake_minimum_required(VERSION 2.6)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
SET(mpsd_SRCS
   main.c
   conf.c
   core.c
   debug.c
   tftp.c
   module.c
   scan.l
)

add_executable(mpsd ${mpsd_SRCS})
target_link_libraries(mpsd)
install(TARGETS mpsd DESTINATION bin)
