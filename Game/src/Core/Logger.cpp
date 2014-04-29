#include "precompiled.h"
#include "Logger.h"

const unsigned int kMaxLogBufferSize = 4096;

void Logger::LogInfo(string info, ...)
{
	char buffer[kMaxLogBufferSize];
	va_list args;
	va_start (args, info);
	vsnprintf(buffer, kMaxLogBufferSize, info.c_str(), args);
	va_end(args);

	Log("\nGameInfo: ", buffer);
}

void Logger::LogError(string error, ...)
{
	char buffer[kMaxLogBufferSize];
	va_list args;
	va_start (args, error);
	vsnprintf(buffer, kMaxLogBufferSize, error.c_str(), args);
	va_end(args);

	Log("\nGameError: ", buffer);
}

void Logger::Log(string type, string output, ...)
{
	OutputDebugStringA((type + output).c_str());
}