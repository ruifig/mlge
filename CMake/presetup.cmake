#
# This file makes it possible to use mlge as a submodule of another repository while still letting mlge cmake scripts setup
# variables in cmake's global scope
#
# The idea is that it allows using mlge as both a submodule or as-is (where the game is put inside the mlge's repo)
# The developer needs to include this file from the root CMakeLists.txt.
# If they don't do that, mlge's cmake script will include the file, which causes an error to be raised explaining what's missing
#

if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	message(FATAL_ERROR "mlge/CMake/presetup.cmake needs to be included from the root CMakeLists.txt")
endif()

set(MLGE_PRESETUP_DONE 1)

#
# Remove the MinSizeRel and RelWithDebInfo configurations and leave it with Debug, Development and Release
#

# Only make changes if we are the top level project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
	if(isMultiConfig)
		if(NOT "Development" IN_LIST CMAKE_CONFIGURATION_TYPES)
			list(APPEND CMAKE_CONFIGURATION_TYPES Development)
		endif()
	else()
		set(allowedBuildTypes Debug Release Development)
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
			STRINGS "${allowedBuildTypes}"
		)
		if(NOT CMAKE_BUILD_TYPE)
			set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
		elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
			message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
		endif()
	endif()

	# Set relevant Development-specific flag variables as needed...

	# Set variables for Development by basing them on Release. NOTE: Add the extra parameters after the ${CMAKExxxx} variable.
	set(CMAKE_C_FLAGS_DEVELOPMENT
	  "${CMAKE_C_FLAGS_RELEASE}"
	  CACHE STRING ""
	)
	set(CMAKE_CXX_FLAGS_DEVELOPMENT
	  "${CMAKE_CXX_FLAGS_RELEASE}"
	  CACHE STRING ""
	)
	set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT
	  "${CMAKE_EXE_LINKER_FLAGS_RELEASE}"
	  CACHE STRING ""
	)
	set(CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT
	  "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}"
	  CACHE STRING ""
	)
	set(CMAKE_STATIC_LINKER_FLAGS_DEVELOPMENT
	  "${CMAKE_STATIC_LINKER_FLAGS_RELEASE}"
	  CACHE STRING ""
	)
	set(CMAKE_MODULE_LINKER_FLAGS_DEVELOPMENT
	  "${CMAKE_MODULE_LINKER_FLAGS_RELEASE}"
	  CACHE STRING ""
	)

	# Remove the Build configurations we don't care about
	list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES MinSizeRel RelWithDebInfo)
endif()

if (MSVC)
	## Enable multi-core building
	add_compile_options(/MP)

	# Add debug information to Release (By default, the "Release" configuration cmake creates has no debug info)
	add_compile_options("$<$<CONFIG:Release>:/Zi>")
	# Add debug information to Release (By default, the "Release" configuration cmake creates has no debug info)
	add_compile_options("$<$<CONFIG:Development>:/Zi>")

	# See https://learn.microsoft.com/en-us/cpp/build/reference/debug-generate-debug-info?view=msvc-170 for an explain of this
	add_link_options("$<$<CONFIG:Debug>:/DEBUG:FULL>")
	add_link_options("$<$<CONFIG:Release>:/DEBUG:FULL>")
	add_link_options("$<$<CONFIG:Development>:/DEBUG:FULL>")
endif()


