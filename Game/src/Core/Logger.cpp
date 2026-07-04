#include "precompiled.h"
#include "Logger.h"

const unsigned int kMaxLogBufferSize = 4096;

void Logger::LogInfo(std::string info, ...)
{
	char buffer[kMaxLogBufferSize];
	va_list args;
	va_start (args, info);
	vsnprintf(buffer, kMaxLogBufferSize, info.c_str(), args);
	va_end(args);

	Log("\nGameInfo: ", buffer);
}

void Logger::LogError(std::string error, ...)
{
	char buffer[kMaxLogBufferSize];
	va_list args;
	va_start (args, error);
	vsnprintf(buffer, kMaxLogBufferSize, error.c_str(), args);
	va_end(args);

	Log("\nGameError: ", buffer);
}

void Logger::Log(std::string type, std::string output, ...)
{
#if defined(DEATHLEAK_PLATFORM_MAC) && DEATHLEAK_PLATFORM_MAC
	std::fprintf(stderr, "%s%s", type.c_str(), output.c_str());
#else
	OutputDebugStringA((type + output).c_str());
#endif
}
