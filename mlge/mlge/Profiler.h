#pragma  once

#include "mlge/Common.h"
#include "crazygaze/core/Singleton.h"

MLGE_THIRD_PARTY_INCLUDES_START
#if MLGE_PROFILER_ENABLED
	#include "Remotery.h"
#endif
MLGE_THIRD_PARTY_INCLUDES_END

namespace mlge
{

/**
 * Starts and shuts down Remotery
 * NOTE: isEnabled is intentionally static for performance reasons.
 */
class Profiler : public Singleton<Profiler>
{
  public:

	Profiler();
	~Profiler();

	/**
	 * Starts the profiler. If any error occurs, it simply leaves the profiler disabled.
	 */
	bool init();

	/**
	 * Instead of requiring to go through he Profiler::get().isEnabled(), the concept of "enabled" is exposed directly for
	 * performance reasons, so the sampling code doesn't need to dereference a pointer to check if the profiler is enabled or not
	 */
	static bool isEnabled()
	{
#if MLGE_PROFILER_ENABLED
		return rmt != nullptr;
#else
		return false;
#endif
	}

  private:

	/**
	 * Remotery instance
	 */
	inline static void* rmt = nullptr;;
};

#if MLGE_PROFILER_ENABLED

/**
 * Terminates a scoped CPU profiling sample
 * Code should use the provided macros instead of using this directly
 *
 * Note that rmt_BeginCPUSample is NOT done in here. It's done by the macro. This is because rmt_BeginCPUSample doesn't take a
 * string as a parameter and creates another static that needs to be outside of ProfileScope.
 * Therefore, this little class is just to automatically end the sample
 */
class ProfileScopeEnd
{
  public:
	ProfileScopeEnd() = default;
	ProfileScopeEnd(const ProfileScopeEnd&) = delete;
	ProfileScopeEnd (ProfileScopeEnd&&) = delete;
	ProfileScopeEnd& operator=(const ProfileScopeEnd&) = delete;
	ProfileScopeEnd& operator=(ProfileScopeEnd&&) = delete;

	inline ~ProfileScopeEnd()
	{
		if (Profiler::isEnabled())
		{
			rmt_EndCPUSample();
		}
	}
};


/**
 * Starts a scoped cpu sample
 */
#define MLGE_PROFILE_SCOPE(name)                                  \
	rmt_BeginCPUSample(name, 0);                                  \
	::mlge::ProfileScopeEnd MLGE_ANONYMOUS_VARIABLE(PROFILER_SCOPE)

/**
 * Starts a scoped cpu sample but using an actual string literal
 */
#define MLGE_PROFILE_SCOPE_DYNAMIC(nameStr)                       \
	rmt_BeginCPUSampleDynamic(nameStr, 0);                        \
	::mlge::ProfileScopeEnd MLGE_ANONYMOUS_VARIABLE(PROFILER_SCOPE)

#define MLGE_PROFILE_SCOPE_EX(name, flags)                        \
	rmt_BeginCPUSample(name, flags);                              \
	::mlge::ProfileScopeEnd MLGE_ANONYMOUS_VARIABLE(PROFILER_SCOPE)

#else // MLGE_PROFILER_ENABLED


#define MLGE_PROFILE_SCOPE(name)
#define MLGE_PROFILE_SCOPE_EX(name, flags)

#endif // MLGE_PROFILER_ENABLED

} // namespace mlge

