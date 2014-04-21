#ifndef LOGGER_H
#define LOGGER_H

#ifndef LOG_INFO_ENABLED
	#ifdef _RELEASE
		#define LOG_INFO_ENABLED 0
	#else
		#define LOG_INFO_ENABLED 1
	#endif
#endif

#ifndef LOG_ERROR_ENABLED
	#define LOG_ERROR_ENABLED 1
#endif

#if LOG_INFO_ENABLED == 1
	
	#define LOG_INFO(...)\
		\
		do\
		{\
		Logger::LogInfo(__VA_ARGS__);\
		\
		} while (0)
#else
	#define LOG_INFO(...)
#endif

#if LOG_ERROR_ENABLED == 1
	
	#define LOG_ERROR(...)\
		\
		do\
		{\
		Logger::LogError(__VA_ARGS__);\
		\
		} while (0)
#else
	#define LOG_ERROR(...)
#endif

class Logger
{
public:

	static void LogInfo(string info, ...);

	static void LogError(string error, ...);

private:

	static void Log(string type, string output, ...);
};

#endif
