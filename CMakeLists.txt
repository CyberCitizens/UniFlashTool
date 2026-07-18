cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0144 OLD)
project(UniversalFlashingTool)

set(REPROC++ ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
	add_compile_options(/W3 /utf-8)
	set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi")
else()
	add_compile_options(-pipe)
	set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASAN_FLAGS}")
	endif()

	set(CMAKE_CXX_FLAGS_DEBUG "-march=native -O0 -g ${ASAN_FLAGS}")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -pipe")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address")
	link_libraries(-fsanitize=address)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ---- Cache persistant pour éviter de rebuild Drogon à chaque clean ----
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.deps_cache" CACHE PATH "")

find_package(LibArchive REQUIRED)
if(TARGET LibArchive::LibArchive)
	set(LIBARCHIVE_TARGET LibArchive::LibArchive)
else()
	set(LIBARCHIVE_TARGET ${LibArchive_LIBRARIES})
	include_directories(${LibArchive_INCLUDE_DIRS})
endif()

# ---- Drogon flags ----
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_ORM OFF CACHE BOOL "" FORCE)          # disabled DB
set(BUILD_CTL OFF CACHE BOOL "" FORCE)          # disabled CLI
set(COZ_PROFILING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)  # static build

# https://github.com/DaanDeMeyer/reproc

FetchContent_Declare(
	reproc
	GIT_REPOSITORY https://github.com/DaanDeMeyer/reproc.git
	GIT_TAG v14.2.7   
	GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(reproc)
FetchContent_Declare(
	drogon
	GIT_REPOSITORY https://github.com/drogonframework/drogon.git
	GIT_TAG v1.9.7   
	GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(drogon)

# ---- Ton exécutable, sans Qt ----
add_executable(uniflashtool
	src/main.cpp
#	src/webserver/HttpServer.cpp
#	src/webserver/HttpServer.hpp
#	src/webserver/WsController.cpp
#	src/webserver/WsController.hpp
#	src/webserver/AuthFilter.cpp
#	src/webserver/AuthFilter.hpp
	src/platform/deps.cpp
	src/renewed/replace/strings_on_steroids.cpp
	src/renewed/interactions.cpp
	src/tools/tools.cpp
	src/tools/flash.cpp
	src/tools/defaults/ROM.cpp
	src/tools/defaults/Recovery.cpp
	src/tools/Config.cpp
)

# macOS bundle si tu veux garder un .app propre
if(APPLE)
	set_target_properties(uniflashtool PROPERTIES
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_BUNDLE_NAME "uniflashtool"
	)
endif()

add_subdirectory(libs/curlpp)
add_subdirectory(libs/pugixml)

target_include_directories(uniflashtool PRIVATE
	curlpp
	pugixml
)

target_link_libraries(uniflashtool PRIVATE
	curlpp
	pugixml
	"${LIBARCHIVE_TARGET}"
	drogon
	reproc++
)

# ---- Copie des assets front (HTML/JS/CSS statiques) à côté du binaire ----
add_custom_command(TARGET uniflashtool POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/web
		$<TARGET_FILE_DIR:uniflashtool>/web
)