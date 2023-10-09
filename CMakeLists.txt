# Minimum CMake version is 3.12
cmake_minimum_required (VERSION 3.12)

# Project name
project (tintirek_project)

# Set defaults for compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


#############################################
#											#
#				PROJECT DEFAULTS			#
#											#
#############################################


# Set sources for each library
set(LIBTRK_CORE_SOURCES
	"tintirek/libtrk_core/trk_core.h"
	"tintirek/libtrk_core/sqlite3.h"
	"tintirek/libtrk_core/crypto.h"
	"tintirek/libtrk_core/config.cpp"
	"tintirek/libtrk_core/crypto.cpp"
	"tintirek/libtrk_core/map.cpp"
	"tintirek/libtrk_core/sqlite3.cpp"
	"tintirek/libtrk_core/string.cpp"
	"tintirek/libtrk_core/version.cpp"
)
set(LIBTRK_DATABASE_SOURCES
	"tintirek/libtrk_database/trk_database.h"
	"tintirek/libtrk_database/database.cpp"
)
set(LIBTRK_CLIENT_SOURCES
	"tintirek/libtrk_client/trk_client.h"
	"tintirek/libtrk_client/client.cpp"
	"tintirek/libtrk_client/cmdline.cpp"
	"tintirek/libtrk_client/connect.h"
	"tintirek/libtrk_client/connect.cpp"
	"tintirek/libtrk_client/passwd.h"
	"tintirek/libtrk_client/passwd.cpp"
)

# Create libraries
add_library(tintirek INTERFACE)
add_library(trk_core SHARED ${LIBTRK_CORE_SOURCES})
add_library(trk_database SHARED ${LIBTRK_DATABASE_SOURCES})
add_library(trk_client SHARED ${LIBTRK_CLIENT_SOURCES})

# Add preprocessor for libraries
target_compile_definitions(trk_core PRIVATE LIBRARY_TRK_CORE)
target_compile_definitions(trk_database PRIVATE LIBRARY_TRK_DATABASE)
target_compile_definitions(trk_client PRIVATE LIBRARY_TRK_CLIENT)

# Define include directories
target_include_directories(tintirek INTERFACE "tintirek/include")
target_include_directories(trk_core PUBLIC "tintirek/include" "tintirek/libtrk_core")
target_include_directories(trk_database PUBLIC "tintirek/include" "tintirek/libtrk_database")
target_include_directories(trk_client PUBLIC "tintirek/include" "tintirek/libtrk_client")

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
	"tintirek/trk/commands/login.h"
	"tintirek/trk/commands/login.cpp"
	"tintirek/trk/commands/logout.h"
	"tintirek/trk/commands/logout.cpp"
	"tintirek/trk/commands/trust.h"
	"tintirek/trk/commands/trust.cpp"
)
# Create trks (server program) executable
add_executable(trks
	"tintirek/trks/trks.cpp"
	"tintirek/trks/logger.h"
	"tintirek/trks/server.h"
	"tintirek/trks/server.cpp"
	"tintirek/trks/service.h"
	"tintirek/trks/Linux/linuxserver.cpp"
	"tintirek/trks/Linux/linuxservice.cpp"
	"tintirek/trks/MacOS/macosserver.cpp"
	"tintirek/trks/MacOS/macosservice.cpp"
	"tintirek/trks/Windows/windowsservice.cpp"
	"tintirek/trks/Windows/windowsserver.cpp"
)

# Link libraries for each executable
target_link_libraries(trk_database PRIVATE trk_core)
target_link_libraries(trk_client PRIVATE trk_core)
target_link_libraries(trk PRIVATE tintirek trk_core trk_client)
target_link_libraries(trks PRIVATE tintirek trk_core trk_database)


#############################################
#											#
#			THIRD PARTY LIBRARIES			#
#											#
#############################################


# SQLite library
add_library(sqlite3 STATIC 
	"deps/sqlite-amalgamation/sqlite3.c"
	"deps/sqlite-amalgamation/sqlite3.h"
	"deps/sqlite-amalgamation/sqlite3ext.h"
)
set_target_properties(sqlite3 PROPERTIES COMPILE_FLAGS "-fPIC")

# OpenSSL library
set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/deps/openssl")
find_package(OpenSSL REQUIRED)

# Add libraries to project
target_link_libraries(trk_core PRIVATE 
	sqlite3
	OpenSSL::SSL
	OpenSSL::Crypto
)
target_link_libraries(trk_client PRIVATE
	OpenSSL::SSL
	OpenSSL::Crypto
)
target_link_libraries(trks PRIVATE
	OpenSSL::SSL
	OpenSSL::Crypto
)
target_link_libraries(trk PRIVATE
	OpenSSL::SSL
	OpenSSL::Crypto
)