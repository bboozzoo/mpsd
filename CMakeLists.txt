cmake_minimum_required(VERSION 2.6)
set(extra_cmake_modules ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(${extra_cmake_modules}/flex.cmake)

# source files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
set(mpsd_SRCS
   main.c
   conf.c
   core.c
   debug.c
   module.c
)
add_flex_files(mpsd_SRCS scan.l)
# options
option(DYNAMIC_PLUGINS
    "Use dynamically loaded protocol plugins"
    OFF)
option(MODULE_TFTP
    "Support TFTP"
    OFF)
# 
if (MODULE_TFTP)
    set(mpsd_SRCS ${mpsd_SRCS} tftp.c)
endif (MODULE_TFTP)
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(mpsd ${mpsd_SRCS})
#target_link_libraries(mpsd)
install(TARGETS mpsd DESTINATION bin)
message(sources: ${mpsd_SRCS})

