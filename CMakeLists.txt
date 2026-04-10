cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0144 OLD)
project(UniversalFlashingTool)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    # Flags for Windows (MSVC)
    add_compile_options(/W3 /utf-8) 
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi") # /Od = no optimization, /Zi = debug infos
    set(CMAKE_CXX_FLAGS_RELEASE "/Od")
else()
    # Flags pour Linux/macOS (GCC/Clang)
    add_compile_options(-pipe -Wno-unknown-argument)
    set(CMAKE_CXX_FLAGS_DEBUG "-march=native -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O0 -pipe")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Gui Concurrent)
# set(LIBXML2_ROOT "${CMAKE_BINARY_DIR}/_deps/libxml2")
# add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
# link_libraries(-fsanitize=address)
# set(LIBXML2_SOURCE_DIR "${CMAKE_SOURCE_DIR}/libs/libxml2")
# set(LIBXML2_BUILD_DIR  "${CMAKE_BINARY_DIR}/_deps/libxml2-build")
# set(LIBXML2_INSTALL_DIR  "${CMAKE_BINARY_DIR}/_deps/libxml2-install")

include(FetchContent)

# FetchContent_Declare(
#     libxml2
#     SOURCE_DIR ${LIBXML2_SOURCE_DIR}
# )

# FetchContent_MakeAvailable(libxml2)

# find_package(LibXml2 REQUIRED)
find_package(LibArchive REQUIRED)

if(TARGET LibArchive::LibArchive)
    set(LIBARCHIVE_TARGET LibArchive::LibArchive)
else()
    set(LIBARCHIVE_TARGET ${LibArchive_LIBRARIES})
    include_directories(${LibArchive_INCLUDE_DIRS})
endif()

qt_standard_project_setup()
qt_add_executable(uniflashtool
	src/main.cpp
	src/gui/UniFlash.cpp
	src/gui/UniFlash.hpp
	src/gui/elements/LabeledWidget.cpp
	src/gui/elements/LabeledWidget.hpp
	src/gui/elements/LayoutElement.cpp
	src/gui/elements/LayoutElement.hpp
	src/gui/elements/ToolWidget.cpp
	src/gui/dialogs/RepoDialog.cpp
	src/gui/dialogs/GitHubTool.cpp
	src/gui/dialogs/GitHubTool.hpp
	src/gui/dialogs/Flash.cpp
	src/platform/deps.cpp
	src/tools/tools.cpp
	src/tools/flash.cpp
	src/tools/defaults/ROM.cpp
	src/tools/defaults/Recovery.cpp
	src/tools/Config.cpp
)
set_target_properties(uniflashtool PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "uniflashtool"
)
add_subdirectory(libs/curlpp)


target_include_directories(uniflashtool PRIVATE
	"${LIBXML2_INCLUDE_DIR}"
)

target_compile_options(uniflashtool PRIVATE -w)

target_link_libraries(uniflashtool PRIVATE 
	curlpp
#	${LIBXML2_LIBRARIES}
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Network
	Qt6::Gui
	Qt6::Concurrent
	"${LIBARCHIVE_TARGET}"
)
