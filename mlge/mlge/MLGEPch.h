#pragma once

#include "mlge/PreSetup.h"

MLGE_THIRD_PARTY_INCLUDES_START

#include <assert.h>
#include <filesystem>
#include <string>
#include <numbers>
#include <set>

#include "mlge/nlohmann/json.hpp"

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_render.h"
#include "SDL_events.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_syswm.h"

#if MLGE_EDITOR
	#define IMGUI_DEFINE_MATH_OPERATORS
	#include "imgui.h"
	#include "misc/cpp/imgui_stdlib.h"
	#include "ImGuiFileDialog.h"
	#include <imgui_internal.h>
#endif

MLGE_THIRD_PARTY_INCLUDES_END

static_assert(SDL_BYTEORDER == SDL_LIL_ENDIAN, "mlge only supports little endian architectures");

namespace mlge
{

using json = nlohmann::json;

} // namespace mlge

