
#
# ScratchpadGame
#

set(ALL_FILES
	"./Assets/ScratchpadGame.assets.json"

	"./Src/PlayerShip.cpp"
	"./Src/PlayerShip.h"
	"./Src/ScratchpadGame.cpp"
	"./Src/ScratchpadGame.h"
	"./Src/ScratchpadGamePch.h"
)

add_executable(ScratchpadGame
	${ALL_FILES}
)

target_link_libraries(ScratchpadGame PRIVATE
	mlge
)

source_group(
	TREE "${CMAKE_CURRENT_LIST_DIR}"
	PREFIX ""
	FILES ${ALL_FILES}
)

set_target_properties(ScratchpadGame PROPERTIES CXX_STANDARD 20)
set_target_properties(ScratchpadGame PROPERTIES FOLDER "mlge/Samples/")
target_compile_options(ScratchpadGame PRIVATE /Wall)

target_precompile_headers(ScratchpadGame PRIVATE "./Src/ScratchpadGamePch.h")

# Setup whatever mlge needs
mlge_setupBinaryTarget(ScratchpadGame)

#
# ScratchpadGameGen
# 
set(GAME_MLGEGEN_FILES
	"${CMAKE_CURRENT_LIST_DIR}/Gen/ScratchpadGameGen.cpp"	

	PARENT_SCOPE
)

