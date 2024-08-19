#include "Logging.h"
#include "StringUtils.h"
#include "LogOutputs.h"

namespace cz
{

namespace details
{

void doDebugBreak()
{
#if defined _WIN32
	__debugbreak();
#elif defined __linux__
	__builtin_trap();
#else
	#error "Uknown or unsupported platform"
#endif
}

LogLevel currMaxLogLevel = compileTimeMaxLogLevel;

static const char* logLevelsStrs[static_cast<int>(LogLevel::VeryVerbose)+1] =
{
	"Off",
	"FTL",
	"ERR",
	"WRN",
	"LOG",
	"VER",
	"VVE"
};

LogLevel logLevelFromString(std::string_view str)
{
	for(int i = 0; i <= static_cast<int>(LogLevel::VeryVerbose); i++)
	{
		if (asciiStrEqualsCi(str, logLevelsStrs[i]))
		{
			return static_cast<LogLevel>(i);
		}
	}

	return LogLevel::Off;
}

/** Don't use this directly. Use the LOG macros */
void logMessage(bool debuggerOutput, LogLevel level, const char* category, const char* msg)
{
	auto nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::utc_clock::now());
	auto nowSecs = std::chrono::time_point_cast<std::chrono::seconds>(nowMs);
	auto ms = nowMs - nowSecs;
	std::string timestamp = std::format("{:%H:%M:%S}:{:03d}", nowSecs, ms.count());

	if (LogOutputs* logs = LogOutputs::tryGet())
	{
		logs->log(debuggerOutput, level, category, timestamp.c_str(), msg);
	}
}

} // namespace details

const char* to_string(LogLevel level)
{
	return details::logLevelsStrs[static_cast<int>(level)];
}

} // namespace cz



