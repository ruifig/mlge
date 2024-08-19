set(ALL_FILES
	"Assets/mlge.assets.json"

	"mlge/Actor.cpp"
	"mlge/Actor.h"
	"mlge/ActorComponent.h"
	"mlge/Common.h"
	"mlge/Config.cpp"
	"mlge/Config.h"
	"mlge/Delegates.cpp"
	"mlge/Delegates.h"
	"mlge/Engine.cpp"
	"mlge/Engine.h"
	"mlge/FlipbookComponent.cpp"
	"mlge/FlipbookComponent.h"
	"mlge/Game.cpp"
	"mlge/Game.h"
	"mlge/GameClock.h"
	"mlge/GlobalConstructorsTouch.cpp"
	"mlge/Level.cpp"
	"mlge/Level.h"
	"mlge/Math.cpp"
	"mlge/Math.h"
	"mlge/MLGEMain.cpp"
	"mlge/MLGEPch.h"
	"mlge/Object.cpp"
	"mlge/Object.h"
	"mlge/ObjectPtr.cpp"
	"mlge/ObjectPtr.h"
	"mlge/Paths.cpp"
	"mlge/Paths.h"
	"mlge/PreSetup.h"
	"mlge/Profiler.cpp"
	"mlge/Profiler.h"
	"mlge/Root.cpp"
	"mlge/Root.h"
	"mlge/Text.cpp"
	"mlge/Text.h"
	"mlge/TextureComponent.cpp"
	"mlge/TextureComponent.h"

	"mlge/Misc/ContainerUtils.h"
	"mlge/Misc/LinkedList.h"

	"mlge/nlohmann/json.hpp"
	"mlge/nlohmann/json_fwd.hpp"
	"mlge/nlohmann/nlohmann_json.natvis"

	"mlge/Render/DXDebugLayer.cpp"
	"mlge/Render/DXDebugLayer.h"
	"mlge/Render/Renderer.cpp"
	"mlge/Render/Renderer.h"
	"mlge/Render/RenderQueue.cpp"
	"mlge/Render/RenderQueue.h"
	"mlge/Render/RenderTarget.cpp"
	"mlge/Render/RenderTarget.h"

	"mlge/Resource/Flipbook.cpp"
	"mlge/Resource/Flipbook.h"
	"mlge/Resource/Image.cpp"
	"mlge/Resource/Image.h"
	"mlge/Resource/Resource.cpp"
	"mlge/Resource/Resource.h"
	"mlge/Resource/SpriteSheet.cpp"
	"mlge/Resource/SpriteSheet.h"
	"mlge/Resource/Texture.cpp"
	"mlge/Resource/Texture.h"
	"mlge/Resource/TTFFont.cpp"
	"mlge/Resource/TTFFont.h"

	"mlge/Editor/AssetBrowser.cpp"
	"mlge/Editor/AssetBrowser.h"
	"mlge/Editor/Console.cpp"
	"mlge/Editor/Console.h"
	"mlge/Editor/Editor.cpp"
	"mlge/Editor/Editor.h"
	"mlge/Editor/GameControlBar.cpp"
	"mlge/Editor/GameControlBar.h"
	"mlge/Editor/GameWindow.cpp"
	"mlge/Editor/GameWindow.h"
	"mlge/Editor/ImGuiLayer.cpp"
	"mlge/Editor/ImGuiLayer.h"
	"mlge/Editor/ResourceWindow.cpp"
	"mlge/Editor/ResourceWindow.h"
	"mlge/Editor/Window.cpp"
	"mlge/Editor/Window.h"

	"mlge/mlge.natvis"
)

add_library(mlge STATIC
	${ALL_FILES}
)

target_link_libraries(mlge PUBLIC
	SDL2::Core
	SDL2::Main
	SDL2::Image
	SDL2::TTF

	czcore

	dxgi
	dxguid
	d3d12

	$<$<OR:$<CONFIG:Debug>,$<CONFIG:Development>>:Remotery>
	$<$<OR:$<CONFIG:Debug>,$<CONFIG:Development>>:imgui>

	timestamp
)

target_include_directories(mlge PUBLIC
	"./"
)

#target_compile_definitions(mlge PUBLIC $<$<CONFIG:Debug>:MLGE_DEBUG=1>)
#target_compile_definitions(mlge PUBLIC $<$<CONFIG:Development>:MLGE_DEVELOPMENT=1>)
#target_compile_definitions(mlge PUBLIC $<$<CONFIG:Release>:MLGE_RELEASE=1>)
target_compile_definitions(mlge PUBLIC $<$<OR:$<CONFIG:Debug>,$<CONFIG:Debug>>:MLGE_PROFILER_ENABLED=0>)
target_compile_definitions(mlge PUBLIC $<$<OR:$<CONFIG:Debug>,$<CONFIG:Development>>:MLGE_PROFILER_ENABLED=0>)
target_compile_definitions(mlge PUBLIC $<$<OR:$<CONFIG:Debug>,$<CONFIG:Development>>:MLGE_EDITOR=1>)

# This is used by the code to figure out where relative to the ./Bin folder the engine and game files are at run time
target_compile_definitions(mlge PRIVATE -DMLGE_BUILD_ROOT="${CMAKE_SOURCE_DIR}")

source_group(
	TREE "${CMAKE_CURRENT_LIST_DIR}"
	PREFIX ""
	FILES ${ALL_FILES}
)

set_target_properties(mlge PROPERTIES CXX_STANDARD 20)
target_compile_options(mlge PRIVATE /Wall)

target_precompile_headers(mlge PRIVATE "mlge/MLGEPch.h")

set_target_properties(mlge PROPERTIES FOLDER "mlge/")

