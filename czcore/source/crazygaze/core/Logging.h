#pragma once

#include "Common.h"

namespace cz
{

enum class LogLevel
{
	Off,
	Fatal,
	Error,
	Warning,
	Log,
	Verbose,
	VeryVerbose
};

const char* to_string(LogLevel level);

namespace details
{

	/**
	 * Breaks into the debugger
	 */
	void doDebugBreak();

	/**
	 * Don't use this directly. Use the LOG macros
	 */
	void logMessage(bool debuggerOutput, LogLevel level, const char* category, const char* msg);

#if CZ_DEBUG
	constexpr LogLevel compileTimeMaxLogLevel = LogLevel::VeryVerbose;
#elif CZ_DEVELOPMENT
	constexpr LogLevel compileTimeMaxLogLevel = LogLevel::VeryVerbose;
#else
	constexpr LogLevel compileTimeMaxLogLevel = LogLevel::Verbose;
#endif

	extern LogLevel currMaxLogLevel;

	LogLevel logLevelFromString(std::string_view str);

} // namespace details


} // namespace cz

#define CZ_LOG_IMPL(debuggerOutput, logLevel, fmtStr, ...)                                                       \
	if constexpr (::cz::LogLevel::logLevel <= ::cz::details::compileTimeMaxLogLevel)                             \
	{                                                                                                            \
		if (::cz::LogLevel::logLevel <= ::cz::details::currMaxLogLevel)                                          \
		{                                                                                                        \
			std::string _cz_internal_msg = std::format(fmtStr, ##__VA_ARGS__);                                   \
			::cz::details::logMessage(debuggerOutput, ::cz::LogLevel::logLevel, "cz", _cz_internal_msg.c_str()); \
			if constexpr (::cz::LogLevel::logLevel == ::cz::LogLevel::Fatal)                                     \
			{                                                                                                    \
				::cz::details::doDebugBreak();                                                                   \
			}                                                                                                    \
		}                                                                                                        \
	}
	
#define CZ_LOG(logLevel, format, ...) CZ_LOG_IMPL(true, logLevel, format,  ##__VA_ARGS__)
#define CZ_LOG_EX(debuggerOutput, logLevel, format, ...) CZ_LOG_IMPL(debuggerOutput, logLevel, format,  ##__VA_ARGS__)

#define CZ_CHECK_IMPL(expr)              \
	if (!(expr))                         \
	{                                    \
		CZ_LOG(Fatal, "Assert :" #expr); \
		exit(0);                         \
	}

#define CZ_CHECK_F_IMPL(expr, format, ...)                           \
	if (!(expr))                                                     \
	{                                                                \
		CZ_LOG(Fatal, "Assert '" #expr "': " format, ##__VA_ARGS__); \
		exit(0);                                                     \
	}

#if CZ_DEBUG
	#define CZ_CHECK(expr) CZ_CHECK_IMPL(expr)
	#define CZ_CHECK_F(expr, format, ...) CZ_CHECK_F_IMPL(expr, format, ##__VA_ARGS__)

	#define CZ_VERIFY(expr) CZ_CHECK_IMPL(expr)
	#define CZ_VERIFY_F(expr, format, ...) CZ_CHECK_F_IMPL(expr, format, ##__VA_ARGS__)
#else
	#define CZ_CHECK(expr) ((void)0)
	#define CZ_CHECK_F(expr, format, ...) ((void)0)

	#define CZ_VERIFY(expr) {if(expr) {}}
	#define CZ_VERIFY_F(expr, format, ...) {if(expr) {}}
#endif


