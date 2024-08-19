cmake_minimum_required(VERSION 3.27)

project(czcore)

option(CZCORE_TESTS "Generate czcore unit tests targets" OFF)

add_subdirectory(source)

if (CZCORE_TESTS)
	if (TARGET Catch2)
		add_subdirectory(./UnitTests)
	else()
		message(FATAL_ERROR "CZCORE_Tests enabled, but no Catch2 target found")
	endif()
endif()

