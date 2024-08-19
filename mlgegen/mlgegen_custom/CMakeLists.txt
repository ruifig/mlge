set(ALL_FILES
	"mlgegen_custom.cpp"
	"mlgegen_custom_Pch.h"
)

add_library(mlgegen_custom STATIC
	${ALL_FILES}
)

#
# Detect if this is the default empty implementation or a user specific one, so we can tweak some things
#
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../mlgegen_custom.h")
	# Default implementation 

	target_include_directories(mlgegen_custom PUBLIC "..")
else()
	# User supplied implementation

	target_include_directories(mlgegen_custom PUBLIC "../mlgegen")
endif()

target_link_libraries(mlgegen_custom PRIVATE
	czcore
)

source_group(
	TREE "${CMAKE_CURRENT_LIST_DIR}"
	PREFIX ""
	FILES ${ALL_FILES}
)

set_target_properties(mlgegen_custom PROPERTIES CXX_STANDARD 20)
target_compile_options(mlgegen_custom PRIVATE /Wall)

target_precompile_headers(mlgegen_custom PRIVATE "mlgegen_custom_Pch.h")
set_target_properties(mlgegen_custom PROPERTIES FOLDER "mlge/")

