#pragma once

#include "CorePreSetup.h"

#define CZ_CONCATENATE_IMPL(s1,s2) s1##s2
#define CZ_CONCATENATE(s1,s2) CZ_CONCATENATE_IMPL(s1,s2)

// Note: __COUNTER__ Expands to an integer starting with 0 and incrementing by 1 every time it is used in a source file or included headers of the source file.
#ifdef __COUNTER__
	#define CZ_ANONYMOUS_VARIABLE(str) \
		CZ_CONCATENATE(str,__COUNTER__)
#else
	#define CZ_ANONYMOUS_VARIABLE(str) \
		CZ_CONCATENATE(str,__LINE__)
#endif


#define CZ_DELETE_COPY_AND_MOVE(Class)         \
	Class(Class&&) = delete;                   \
	Class(const Class&) = delete;              \
	Class& operator=(Class&&) = delete;        \
	Class& operator=(const Class&&) = delete;

