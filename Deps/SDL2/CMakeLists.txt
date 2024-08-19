
if (WIN32)
    # all good
elseif(UNIX)
	message(FATAL_ERROR "Linux not support")
elseif(APPLE)
	message(FATAL_ERROR "MacOSX not support")
else()
	message(FATAL_ERROR "Unknown platform")
endif()

if(CMAKE_SIZEOF_VOID_P STREQUAL "4")
    set(_sdl_arch_subdir "x86")
elseif(CMAKE_SIZEOF_VOID_P STREQUAL "8")
    set(_sdl_arch_subdir "x64")
else()
	message(FATAL_ERROR "Unknown SIZEOF_VOID_P")
endif()

#
# SDL2 Core
#
add_library(SDL2::Core SHARED IMPORTED GLOBAL)
set_target_properties(SDL2::Core PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/Core/include"

	IMPORTED_IMPLIB   "${CMAKE_CURRENT_LIST_DIR}/Core/lib/${_sdl_arch_subdir}/SDL2.lib"
	IMPORTED_LOCATION   "${CMAKE_CURRENT_LIST_DIR}/Core/lib/${_sdl_arch_subdir}/SDL2.dll"
)

#
# SDL2 Main
#
add_library(SDL2::Main STATIC IMPORTED GLOBAL)
set_target_properties(SDL2::Main PROPERTIES
	IMPORTED_LOCATION   "${CMAKE_CURRENT_LIST_DIR}/Core/lib/${_sdl_arch_subdir}/SDL2main.lib"
)

#
# SDL2 Image
#
add_library(SDL2::Image SHARED IMPORTED GLOBAL)
set_target_properties(SDL2::Image PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/Image/include"

	IMPORTED_IMPLIB   "${CMAKE_CURRENT_LIST_DIR}/Image/lib/${_sdl_arch_subdir}/SDL2_image.lib"
	IMPORTED_LOCATION   "${CMAKE_CURRENT_LIST_DIR}/Image/lib/${_sdl_arch_subdir}/SDL2_image.dll"
)

#
# SDL2 TTF
#
add_library(SDL2::TTF SHARED IMPORTED GLOBAL)
set_target_properties(SDL2::TTF PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/TTF/include"

	IMPORTED_IMPLIB   "${CMAKE_CURRENT_LIST_DIR}/TTF/lib/${_sdl_arch_subdir}/SDL2_ttf.lib"
	IMPORTED_LOCATION   "${CMAKE_CURRENT_LIST_DIR}/TTF/lib/${_sdl_arch_subdir}/SDL2_ttf.dll"
)

