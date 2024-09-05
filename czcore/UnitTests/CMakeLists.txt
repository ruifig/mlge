cmake_minimum_required(VERSION 3.27)

set(ALL_FILES
	"CoreUnitTests.cpp"
	"CoreUnitTestsPch.h"
	"SharedPtrTests.cpp"
)

if (WIN32)
	list(APPEND ALL_FILES
		"Win32EventTests.cpp"
	)
endif()

add_executable(CoreUnitTests ${ALL_FILES})

source_group(
	TREE "${CMAKE_CURRENT_LIST_DIR}"
	PREFIX ""
	FILES ${ALL_FILES}
)

set_target_properties(CoreUnitTests PROPERTIES CXX_STANDARD 20)
target_compile_options(CoreUnitTests PRIVATE /Wall)
set_target_properties(CoreUnitTests PROPERTIES FOLDER "${CZ_BASE_IDE_FOLDER}czcore/")

if (TARGET Catch2)
	target_link_libraries(CoreUnitTests PRIVATE Catch2)
else()
	message(FATAL_ERROR "CoreUnitTests enabled, but no Catch2 target found")
endif()

target_link_libraries(CoreUnitTests PUBLIC
	czcore
)

target_precompile_headers(CoreUnitTests PRIVATE "CoreUnitTestsPch.h")

