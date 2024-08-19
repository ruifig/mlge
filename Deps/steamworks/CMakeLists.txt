#
# Tidbits of information:
# https://stackoverflow.com/questions/46360959/how-do-i-specify-the-include-directory-for-an-imported-shared-library-using-cmak
#

add_library(steamworks SHARED IMPORTED GLOBAL)

# Set the include path
set_target_properties(steamworks PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/public")

#
# Useful information about setting up a SHARED IMPORTED library
# Below, I'm used IMPORTED_IMPLIB and IMPORTED_LOCATION, which specifies the libraries for all configurations.
# To specify different libs for Debug and Release, we can do use IMPORTED_IMPLIB_DEBUG/IMPORTED_LOCATION_DEBUG and
# IMPORTED_IMPLIB_RELEASE/IMPORTED_LOCATION_RELEASE
#

if (WIN32)
	set_property(TARGET steamworks PROPERTY IMPORTED_IMPLIB   "${CMAKE_CURRENT_LIST_DIR}/redistributable_bin/win64/steam_api64.lib")
	set_property(TARGET steamworks PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/redistributable_bin/win64/steam_api64.dll")
elseif(UNIX)
	set_property(TARGET steamworks PROPERTY IMPORTED_IMPLIB   "${CMAKE_CURRENT_LIST_DIR}/redistributable_bin/linux64/libsteam_api.so")
	set_property(TARGET steamworks PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/redistributable_bin/linux64/libsteam_api.so")
elseif(APPLE)
	message(FATAL_ERROR "MacOSX not supported yet")
endif()

