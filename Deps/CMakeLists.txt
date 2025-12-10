include(FetchContent)

#
# utfcpp
# This needs to be declared BEFORE czcore, otherwise czcore will download it instead of using what we have in the engine.
FetchContent_Declare(utfcpp
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/utfcpp
)
# utfcpp is a target of type INTERFACE, so it doesn't show in Visual Studio. To work around this we create a custom target.
# The custom target doesn't build anything, but allows us to see the utfcpp files in VS
add_custom_target(utfcpp_ SOURCES
	"utfcpp/source/utf8.h"
	"utfcpp/source/utf8/checked.h"
	"utfcpp/source/utf8/core.h"
	"utfcpp/source/utf8/cpp11.h"
	"utfcpp/source/utf8/cpp17.h"
	"utfcpp/source/utf8/cpp20.h"
	"utfcpp/source/utf8/unchecked.h"
)

# Set in what folder it shows up in the IDE
set_target_properties(utfcpp_ PROPERTIES FOLDER "mlge/Dependencies/")


#
# Catch2
# This needs to be declared BEFORE czcore, otherwise czcore will download it instead of using what we have in the engine.
FetchContent_Declare(Catch2
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Catch2
)

#
# czcore
#
# Note to myself: By default, we use the built-in czcore, but if I want to use an external czcore so I can
# share it with other project while developing czcore, then an override can be provided with a cache variable.
# See:  https://cmake.org/cmake/help/latest/module/FetchContent.html, specifically the part that mentions
# FETCH_CONTENT_SOURCE_DIR
#
FetchContent_Declare(czcore
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/czcore
)
FetchContent_MakeAvailable(czcore)
set_target_properties(czcore PROPERTIES FOLDER "mlge/Dependencies/")
if (TARGET czcore_UnitTests)
	set_target_properties(czcore_UnitTests PROPERTIES FOLDER "mlge/Dependencies/")
endif()
if (TARGET Catch2)
	set_target_properties(Catch2 PROPERTIES FOLDER "mlge/Dependencies/")
	set_target_properties(Catch2WithMain PROPERTIES FOLDER "mlge/Dependencies/")
endif()

#
# SDL2
#
add_subdirectory(./SDL2)
# Add a custom target for SDL so that we can see the headers in Visual Studio.
# The custom target doesn't build anything, but allows us to see the utfcpp files in VS
file(GLOB_RECURSE SDLSOURCES "SDL2/Core/include/*.*" "SDL2/Image/include/*.*" "SDL2/TTF/include/*.*")
add_custom_target(SDL_ SOURCES ${SDLSOURCES})
# Set in what folder it shows up in the IDE
set_target_properties(SDL_ PROPERTIES FOLDER "mlge/Dependencies/")

#
# Setup imgui
#
add_library(imgui STATIC
	"imgui/imconfig.h"
	"imgui/imgui.cpp"
	"imgui/imgui.h"
	"imgui/misc/debuggers/imgui.natstepfilter"
	"imgui/misc/debuggers/imgui.natvis"
	"imgui/imgui_draw.cpp"
	"imgui/imgui_internal.h"
	"imgui/imgui_tables.cpp"
	"imgui/imgui_widgets.cpp"

	"imgui/imgui_demo.cpp"

	"imgui/backends/imgui_impl_sdl2.cpp"
	"imgui/backends/imgui_impl_sdl2.h"
	"imgui/backends/imgui_impl_sdlrenderer2.cpp"
	"imgui/backends/imgui_impl_sdlrenderer2.h"

	"imgui/misc/cpp/imgui_stdlib.cpp"
	"imgui/misc/cpp/imgui_stdlib.h"

	"ImGuiFileDialog/ImGuiFileDialog.cpp"
	"ImGuiFileDialog/ImGuiFileDialog.h"
	"ImGuiFileDialog/ImGuiFileDialogConfig.h"

)
target_link_libraries(imgui PRIVATE
	SDL2::Core
	SDL2::Main
)
target_include_directories(imgui PUBLIC "imgui/")
target_include_directories(imgui PUBLIC "ImGuiFileDialog/")
set_target_properties(imgui PROPERTIES FOLDER "mlge/Dependencies/")

add_subdirectory("./steamworks")

#
# cmake_timestamp
#
add_subdirectory(./cmake_timestamp)
set_target_properties(timestamp PROPERTIES FOLDER "mlge/Dependencies/cmake_timestamp")
set_target_properties(clear_cache PROPERTIES FOLDER "mlge/Dependencies/cmake_timestamp")

