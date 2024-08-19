#pragma once

#include "crazygaze/core/CorePreSetup.h"

namespace mlge
{
	using namespace cz;

	namespace details
	{
		using namespace cz::details;
	}
}

#define MLGE_THIRD_PARTY_INCLUDES_START CZ_THIRD_PARTY_INCLUDES_START
#define MLGE_THIRD_PARTY_INCLUDES_END CZ_THIRD_PARTY_INCLUDES_END

#ifndef MLGE_EDITOR
	#define MLGE_EDITOR 0
#endif

#ifndef MLGE_PROFILER_ENABLED
	#define MLGE_PROFILER_ENABLED 0
#endif

#define MLGE_DEBUG CZ_DEBUG
#define MLGE_DEVELOPMENT CZ_DEVELOPMENT
#define MLGE_RELEASE CZ_RELEASE
