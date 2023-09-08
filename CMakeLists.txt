# Minimum CMake version is 3.12
cmake_minimum_required (VERSION 3.12)

# Project name
project (tintirek_project)

# Set defaults for compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Set sources for each library
set(LIBTRK_CLIENT_SOURCES
	"tintirek/libtrk_client/trk_client.h"
	"tintirek/libtrk_client/connect.h"
	"tintirek/libtrk_client/connect.cpp"
	"tintirek/libtrk_client/cmdline.cpp"
)
set(LIBTRK_CORE_SOURCES
	"tintirek/libtrk_core/trk_core.h"
	"tintirek/libtrk_core/config.cpp"
	"tintirek/libtrk_core/string.cpp"
	"tintirek/libtrk_core/version.cpp"
)

# Create libraries
add_library(tintirek INTERFACE)
add_library(trk_client SHARED ${LIBTRK_CLIENT_SOURCES})
add_library(trk_core SHARED ${LIBTRK_CORE_SOURCES})

# Define include directories
target_include_directories(trk_client PUBLIC "tintirek/include" "tintirek/libtrk_client")
target_include_directories(trk_core PUBLIC "tintirek/include" "tintirek/libtrk_core")
target_include_directories(tintirek INTERFACE "tintirek/include")

# Create trk (client program) executable
add_executable (trk 
	"tintirek/trk/trk.cpp"
	"tintirek/trk/commandline.h"
	"tintirek/trk/commands/add.h"
	"tintirek/trk/commands/add.cpp"
	"tintirek/trk/commands/edit.h"
	"tintirek/trk/commands/edit.cpp"
	"tintirek/trk/commands/info.h"
	"tintirek/trk/commands/info.cpp"
)

# Create trks (server program) executable
add_executable(trks
	"tintirek/trks/trks.cpp"
	"tintirek/trks/logger.h"
	"tintirek/trks/server.h"
	"tintirek/trks/server.cpp"
	"tintirek/trks/service.h"
	"tintirek/trks/Linux/linuxserver.cpp"
	"tintirek/trks/Windows/windowsserver.cpp"
	"tintirek/trks/Linux/linuxservice.cpp"
	"tintirek/trks/Windows/windowsservice.cpp"
)

# Link libraries for each executable
target_link_libraries(trk_client PRIVATE trk_core)
target_link_libraries(trk PRIVATE tintirek trk_core trk_client)
target_link_libraries(trks PRIVATE tintirek trk_core)