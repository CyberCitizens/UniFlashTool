cmake_minimum_required(VERSION 3.11)
project(UniversalFlashingTool)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Gui)
qt_standard_project_setup()
qt_add_executable(uniflashtool
	src/main.cpp
	src/gui/UniFlash.cpp
	src/gui/UniFlash.hpp
	src/gui/elements/LabeledWidget.cpp
	src/gui/elements/LabeledWidget.hpp
	src/gui/dialogs/RepoDialog.cpp
	src/gui/dialogs/GitHubTool.cpp
	src/gui/dialogs/GitHubTool.hpp
	src/platform/deps.cpp
	src/tools/tools.cpp
	src/tools/flash.cpp
)
add_subdirectory(libs/curlpp)

add_compile_options(uniflashtool -g)
add_definitions(-w)

target_link_libraries(uniflashtool PRIVATE 
	curlpp
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Network
	Qt6::Gui
)
