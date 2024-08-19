#
# mlgeSample
#

set(ALL_FILES
	"./Assets/mlgeSample.assets.json"
	"./Src/mlgeSample.cpp"
	"./Src/mlgeSample.h"
	"./Src/mlgeSamplePch.h"
)

add_executable(mlgeSample
	${ALL_FILES}
)

target_link_libraries(mlgeSample PRIVATE
	mlge
)

source_group(
	TREE "${CMAKE_CURRENT_LIST_DIR}"
	PREFIX ""
	FILES ${ALL_FILES}
)

set_target_properties(mlgeSample PROPERTIES CXX_STANDARD 20)
set_target_properties(mlgeSample PROPERTIES FOLDER "Samples/")
target_compile_options(mlgeSample PRIVATE /Wall)

target_precompile_headers(mlgeSample PRIVATE "./Src/mlgeSamplePch.h")

set_target_properties(mlgeSample PROPERTIES FOLDER "mlge/Samples")
mlge_setupBinaryTarget(mlgeSample)
#mlge_utils_printTargetProperties(mlgeSample)

