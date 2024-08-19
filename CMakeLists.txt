cmake_minimum_required(VERSION 3.27)

project(mlge)

include(CMake/utils.cmake)

if(NOT MLGE_PRESETUP_DONE)
	include(CMake/presetup.cmake)
endif()

option(MLGE_SAMPLES "Generate mlge sample targets" OFF)
option(MLGE_TESTS "Generate mlge unit tests targets" OFF)

set(CZ_BASE_IDE_FOLDER "mlge/")

add_subdirectory(./Deps)
add_subdirectory(./czcore)

message(STATUS "MLGE : Looking for custom mlgegen library at '${CMAKE_CURRENT_LIST_DIR}/mlgegen_custom'...")
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/mlgegen_custom")
	message(STATUS "MLGE : Custom mlgegen library detected (mlgegen_custom). Using it.")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/mlgegen_custom")
else()
	message(STATUS "MLGE : No custom mlgegen library detected. Using defaults")
	add_subdirectory(./mlgegen/mlgegen_custom)
endif()
add_subdirectory(./mlgegen)

add_subdirectory(./mlge)


if (MLGE_SAMPLES)
	add_subdirectory(./mlgeSample)
endif()



