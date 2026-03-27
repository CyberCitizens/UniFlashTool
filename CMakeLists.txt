cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0144 OLD)
project(UniversalFlashingTool)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-pipe -Wno-unknown-argument)
set(CMAKE_CXX_FLAGS_DEBUG "-march=native -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-march=native -O3 -pipe")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Gui)
set(LIBXML2_ROOT "${CMAKE_BINARY_DIR}/_deps/libxml2")

set(LIBXML2_SOURCE_DIR "${CMAKE_SOURCE_DIR}/libs/libxml2")
set(LIBXML2_BUILD_DIR  "${CMAKE_BINARY_DIR}/_deps/libxml2-build")
set(LIBXML2_INSTALL_DIR  "${CMAKE_BINARY_DIR}/_deps/libxml2-install")

include(FetchContent)

FetchContent_Declare(
    libxml2
    SOURCE_DIR ${LIBXML2_SOURCE_DIR}
)

FetchContent_MakeAvailable(libxml2)

# add_subdirectory("${LIBXML2_ROOT}")

qt_standard_project_setup()
qt_add_executable(uniflashtool
	src/main.cpp
	src/gui/UniFlash.cpp
	src/gui/UniFlash.hpp
	src/gui/elements/LabeledWidget.cpp
	src/gui/elements/LabeledWidget.hpp
	src/gui/elements/LayoutElement.cpp
	src/gui/elements/LayoutElement.hpp
	src/gui/dialogs/RepoDialog.cpp
	src/gui/dialogs/GitHubTool.cpp
	src/gui/dialogs/GitHubTool.hpp
	src/platform/deps.cpp
	src/tools/tools.cpp
	src/tools/flash.cpp
	src/tools/defaults/ROM.cpp
	src/tools/defaults/Recovery.cpp
	src/tools/Config.cpp
)
add_subdirectory(libs/curlpp)

find_package(LibXml2 REQUIRED)

target_include_directories(uniflashtool PRIVATE "${LIBXML2_INCLUDE_DIR}")

target_compile_options(uniflashtool PRIVATE -w)

target_link_libraries(uniflashtool PRIVATE 
	curlpp
	${LIBXML2_LIBRARIES}
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Network
	Qt6::Gui
)
