#
# Add catch2 if tests are enabled, and tweak some things
#
if (MLGE_TESTS OR CZCORE_TESTS)
	add_subdirectory(./Catch2)
	# Set in what folder it shows up in the IDE
	set_target_properties(Catch2 PROPERTIES FOLDER "mlge/Dependencies/")
	set_target_properties(Catch2WithMain PROPERTIES FOLDER "mlge/Dependencies/")
endif()

#
# utfcpp
#
add_subdirectory(./utfcpp)
# utfcpp is a target of type INTERFACE, so it doesn't show in Visual Studio. To work around this we create a custom target.
# The custom target doesn't build anything, but allows us to see the utfcpp files in VS
add_custom_target(utfcpp_ SOURCES
	"utfcpp/source/utf8.h"
	"utfcpp/source/utf8/checked.h"
	"utfcpp/source/utf8/core.h"
	"utfcpp/source/utf8/cpp11.h"
	"utfcpp/source/utf8/cpp17.h"
	"utfcpp/source/utf8/unchecked.h"
)
# Set in what folder it shows up in the IDE
set_target_properties(utfcpp_ PROPERTIES FOLDER "mlge/Dependencies/")

# Add a custom target for SDL so that we can see the headers in Visual Studio.
# The custom target doesn't build anything, but allows us to see the utfcpp files in VS
file(GLOB_RECURSE SDLSOURCES "SDL2/Core/include/*.*" "SDL2/Image/include/*.*" "SDL2/TTF/include/*.*")
add_custom_target(SDL_ SOURCES ${SDLSOURCES})
# Set in what folder it shows up in the IDE
set_target_properties(SDL_ PROPERTIES FOLDER "mlge/Dependencies/")

#
# Setup the Remotery cmake target
#
add_library(Remotery STATIC
	"Remotery/lib/Remotery.c"
	"Remotery/lib/Remotery.h"
)
target_include_directories(Remotery PUBLIC "Remotery/lib")
set_target_properties(Remotery PROPERTIES FOLDER "mlge/Dependencies/")
if(MSVC)
	target_compile_options(Remotery PRIVATE /wd4114)
endif()

#
# SDL2
#
add_subdirectory(./SDL2)

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

add_subdirectory(./cmake_timestamp)
set_target_properties(timestamp PROPERTIES FOLDER "mlge/Dependencies/cmake_timestamp")
set_target_properties(clear_cache PROPERTIES FOLDER "mlge/Dependencies/cmake_timestamp")

