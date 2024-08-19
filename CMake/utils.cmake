###############################################################################################################
#
# Generic utility functions (not necessarily specific to mlge)
#
###############################################################################################################

#
# Copied from https://stackoverflow.com/questions/32183975/how-to-print-all-the-properties-of-a-target-in-cmake
#
#

# Get all properties that cmake supports
if(NOT CMAKE_PROPERTY_LIST)
	execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)
	
	# Convert command output into a CMake list
	string(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
	string(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
	list(REMOVE_DUPLICATES CMAKE_PROPERTY_LIST)
endif()

function(mlge_utils_printProperties)
	message(STATUS "CMAKE_PROPERTY_LIST = ${CMAKE_PROPERTY_LIST}")
endfunction()
	
# Logs all of a target's properties
function(mlge_utils_printTargetProperties target)
	if(NOT TARGET ${target})
		message(STATUS "There is no target named '${target}'")
		return()
	endif()

	foreach(property ${CMAKE_PROPERTY_LIST})
		string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" property ${property})

		# Fix https://stackoverflow.com/questions/32197663/how-can-i-remove-the-the-location-property-may-not-be-read-from-target-error-i
		if(property STREQUAL "LOCATION" OR property MATCHES "^LOCATION_" OR property MATCHES "_LOCATION$")
			continue()
		endif()

		get_property(was_set TARGET ${target} PROPERTY ${property} SET)
		if(was_set)
			get_target_property(value ${target} ${property})
			message(STATUS "${target} ${property} = ${value}")
		endif()
	endforeach()
endfunction()


# Copied from https://stackoverflow.com/questions/14089284/copy-all-dlls-that-an-executable-links-to-to-the-executable-directory/73550650#73550650 
#
# Setups the POST_BUILD command to copy a target's dependencies into the runtime directory
#
function(mlge_utils_copyRuntimeDlls TARGET)
  get_property(already_applied TARGET "${TARGET}" PROPERTY _copy_runtime_dlls_applied)

  if (CMAKE_IMPORT_LIBRARY_SUFFIX AND NOT already_applied)
    add_custom_command(
      TARGET "${TARGET}" POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy
        "$<TARGET_RUNTIME_DLLS:${TARGET}>" "$<TARGET_FILE_DIR:${TARGET}>"
      COMMAND_EXPAND_LISTS
    )
 
    set_property(TARGET "${TARGET}" PROPERTY _copy_runtime_dlls_applied 1)
  endif ()
endfunction()

#
# Sets "var" to a string with the platform name.
# This string can then be used to name platform specific folders or files
#
macro(mlge_utils_getPlatformName var)
	if (WIN32)
		if (CMAKE_SIZEOF_VOID_P EQUAL 8)
			SET( ${var} "Win64")
		else (CMAKE_SIZEOF_VOID_P EQUAL 8)
			SET( ${var} "Win32")
		endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
	elseif(LINUX)
			SET( ${var} "Linux")
	else()
		error("Unsupported platform")
	endif()
endmacro()

#
# Setups an executable target output folder to "Bin" and the name to "<TargetName>-<Platform>-<Config>"
#
function(mlge_utils_setBinaryTargetOutput targetName)
	mlge_utils_getPlatformName(platformName)

	set(subFolder "")

	set_target_properties( ${targetName}
		PROPERTIES

		#ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
		#LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib

		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin${subFolder}$<$<CONFIG:DummyConfigName>:>
		VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin${subFolder}$<$<CONFIG:DummyConfigName>:>

		# Output file name
		OUTPUT_NAME ${targetName}-${platformName}-$<CONFIG>
	)
endfunction()


###############################################################################################################
#
# MLGE functions
#
###############################################################################################################

#
# Setups multiple things for a target that is using mlge:
# - Sets the output directory to "./Bin"
# - Sets the output file to be "<TargetName>-<Platform>-<Config>"
# - Setups the POST_BUILD so mlge dependencies are copied over to the Bin folder
#
function(mlge_setupBinaryTarget targetName)
	mlge_utils_setBinaryTargetOutput(${targetName})
	mlge_utils_copyRuntimeDlls(${targetName})
endfunction()

