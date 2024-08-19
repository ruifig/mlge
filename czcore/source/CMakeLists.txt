set(ALL_FILES
	"crazygaze/core/Algorithm.h"
	"crazygaze/core/AsyncCommandQueue.cpp"
	"crazygaze/core/AsyncCommandQueue.h"
	"crazygaze/core/CommandLine.cpp"
	"crazygaze/core/CommandLine.h"
	"crazygaze/core/Common.h"
	"crazygaze/core/CorePch.h"
	"crazygaze/core/CorePreSetup.h"
	"crazygaze/core/File.cpp"
	"crazygaze/core/File.h"
	"crazygaze/core/IniFile.cpp"
	"crazygaze/core/IniFile.h"
	"crazygaze/core/Logging.cpp"
	"crazygaze/core/Logging.h"
	"crazygaze/core/LogOutputs.cpp"
	"crazygaze/core/LogOutputs.h"
	"crazygaze/core/Misc.h"
	"crazygaze/core/PlatformUtils.cpp"
	"crazygaze/core/PlatformUtils.h"
	"crazygaze/core/ScopeGuard.h"
	"crazygaze/core/Semaphore.cpp"
	"crazygaze/core/Semaphore.h"
	"crazygaze/core/SharedPtr.h"
	"crazygaze/core/SharedQueue.h"
	"crazygaze/core/Singleton.h"
	"crazygaze/core/StringUtils.cpp"
	"crazygaze/core/StringUtils.h"
)

add_library(czcore STATIC ${ALL_FILES})

target_link_libraries(czcore PUBLIC
	utf8cpp
)

target_include_directories(czcore PUBLIC
	"./"
)

source_group(
	TREE "${CMAKE_CURRENT_LIST_DIR}"
	PREFIX ""
	FILES ${ALL_FILES}
)

set_target_properties(czcore PROPERTIES CXX_STANDARD 20)
target_compile_options(czcore PRIVATE /Wall)
set_target_properties(czcore PROPERTIES FOLDER "${CZ_BASE_IDE_FOLDER}czcore/")
target_precompile_headers(czcore PUBLIC "crazygaze/core/CorePch.h")

target_compile_definitions(czcore PUBLIC $<$<CONFIG:Debug>:CZ_DEBUG=1>)
target_compile_definitions(czcore PUBLIC $<$<CONFIG:Release>:CZ_RELEASE>)
target_compile_definitions(czcore PUBLIC $<$<CONFIG:Development>:CZ_DEVELOPMENT=1>)

