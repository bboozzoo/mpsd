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
   conn.c
)
add_flex_files(mpsd_SRCS scan.l)
# options
option(DYNAMIC_PLUGINS
    "Use dynamically loaded protocol plugins"
    OFF)
option(MODULE_TFTP
    "Support TFTP"
    OFF)
option(ENABLE_DEBUG_SYMBOLS
    "Enable debugging symbols"
    ON)
# 
if (NOT DYNAMIC_PLUGINS)
    set(mpsd_SRCS ${mpsd_SRCS} core_static.c)
endif (NOT DYNAMIC_PLUGINS)

if (MODULE_TFTP)
    set(mpsd_SRCS ${mpsd_SRCS} tftp.c)
endif (MODULE_TFTP)

if (ENABLE_DEBUG_SYMBOLS)
    set(CMAKE_BUILD_TYPE DEBUG)
endif(ENABLE_DEBUG_SYMBOLS)
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(mpsd ${mpsd_SRCS})
#target_link_libraries(mpsd)
install(TARGETS mpsd DESTINATION bin)
message(sources: ${mpsd_SRCS})

