#pragma once

#include "mlge/PreSetup.h"

#define CZ_DELETE_COPY_AND_MOVE(Class)       \
	Class(Class&&) = delete;                   \
	Class(const Class&) = delete;              \
	Class& operator=(Class&&) = delete;        \
	Class& operator=(const Class&&) = delete; 

namespace mlge
{

struct SDLDeleter
{
	void operator()(SDL_Window* obj) const
	{
		SDL_DestroyWindow(obj);
	}
	void operator()(SDL_Renderer* obj) const
	{
		SDL_DestroyRenderer(obj);
	}
	void operator()(SDL_Surface* obj) const
	{
		SDL_FreeSurface(obj);
	}
	void operator()(SDL_Texture* obj) const
	{
		SDL_DestroyTexture(obj);
	}
	void operator()(TTF_Font* obj) const
	{
		TTF_CloseFont(obj);
	}
};

template<typename SDLType>
using SDLUniquePtr = std::unique_ptr<SDLType, SDLDeleter>;

#if MLGE_EDITOR
	// If launching an editor build with -game, then this is set to true.
	extern bool gIsGame;
#else
	// If it's not an editor build, then use a macro, so any conditional code gets compiled out
	#define gIsGame true
#endif

} // namespace mlge

#define MLGE_CONCATENATE_IMPL(s1,s2) s1##s2
#define MLGE_CONCATENATE(s1,s2) MLGE_CONCATENATE_IMPL(s1,s2)

// Note: __COUNTER__ Expands to an integer starting with 0 and incrementing by 1 every time it is used in a source file or included headers of the source file.
#ifdef __COUNTER__
	#define MLGE_ANONYMOUS_VARIABLE(str) \
		MLGE_CONCATENATE(str,__COUNTER__)
#else
	#define MLGE_ANONYMOUS_VARIABLE(str) \
		MLGE_CONCATENATE(str,__LINE__)
#endif
