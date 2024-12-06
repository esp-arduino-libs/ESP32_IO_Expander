# This file is to be given as "make USER_C_MODULES=..." when building Micropython port

add_library(usermod_esp_io_expander INTERFACE)

# Set the source directorya and find all source files.
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
file(GLOB_RECURSE SRCS ${SRC_DIR}/*.c)

# Add our source files to the library.
target_sources(usermod_esp_io_expander INTERFACE ${SRCS})

# Add the current directory as an include directory.
target_include_directories(usermod_esp_io_expander INTERFACE ${SRC_DIR})

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_esp_io_expander)
