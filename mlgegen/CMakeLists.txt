set(ALL_FILES
	"Context.h"
	"mlgegen.cpp"
	"mlgegen_custom.h"
	"mlgegenPch.h"
	"parser.h"
	"parser.h"
)

add_executable(mlgegen
	${ALL_FILES}
)

target_link_libraries(mlgegen PRIVATE
	mlgegen_custom
	czcore
)

source_group(
	TREE "${CMAKE_SOURCE_DIR}"
	PREFIX ""
	FILES ${ALL_FILES}
)

set_target_properties(mlgegen PROPERTIES CXX_STANDARD 20)
target_compile_options(mlgegen PRIVATE /Wall)
target_precompile_headers(mlgegen PRIVATE "mlgegenPch.h")
set_target_properties(mlgegen PROPERTIES FOLDER "mlge/")
mlge_utils_setBinaryTargetOutput(mlgegen)


