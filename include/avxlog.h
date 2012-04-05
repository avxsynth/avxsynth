#ifndef __AVX_LOG_H__
#define __AVX_LOG_H__

#include <iostream>
#include <log4cpp/Category.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/SimpleLayout.hh>
using namespace log4cpp;

namespace avxsynth
{
	
#define MAX_VARARGS_LEN	(512)

#define MAKE_STRING(x)  #x
#define STRINGIZE(x) 	MAKE_STRING(x)
#define LINE_STRING     STRINGIZE(__LINE__)

#define DELIMITER       ","

#define FROM(x)	"Module=" MAKE_STRING(x) DELIMITER "Address=" __FILE__ ":" LINE_STRING


#define AVXLOG_DEBUG(format,  ...) 	AvxLog::Debug (FROM(MODULE_NAME), format, __VA_ARGS__);
#define AVXLOG_INFO(format,   ...) 	AvxLog::Info  (FROM(MODULE_NAME), format, __VA_ARGS__);
#define AVXLOG_NOTICE(format, ...) 	AvxLog::Notice(FROM(MODULE_NAME), format, __VA_ARGS__);
#define AVXLOG_WARN(format,   ...) 	AvxLog::Warn  (FROM(MODULE_NAME), format, __VA_ARGS__);
#define AVXLOG_ERROR(format,  ...) 	AvxLog::Error (FROM(MODULE_NAME), format, __VA_ARGS__);
#define AVXLOG_CRIT(format,   ...)	AvxLog::Crit  (FROM(MODULE_NAME), format, __VA_ARGS__);
#define AVXLOG_ALERT(format,  ...) 	AvxLog::Alert (FROM(MODULE_NAME), format, __VA_ARGS__);
#define AVXLOG_FATAL(format,  ...) 	AvxLog::Fatal (FROM(MODULE_NAME), format, __VA_ARGS__);

class AvxLog
{
public:
	static void Debug (const char* pStrModule, const char* pStrformat, ...);
	static void Info  (const char* pStrModule, const char* pStrformat, ...);
	static void Notice(const char* pStrModule, const char* pStrformat, ...);
	static void Warn  (const char* pStrModule, const char* pStrformat, ...);
	static void Error (const char* pStrModule, const char* pStrformat, ...);
	static void Crit  (const char* pStrModule, const char* pStrformat, ...);
	static void Alert (const char* pStrModule, const char* pStrformat, ...);
	static void Fatal (const char* pStrModule, const char* pStrformat, ...);
	
private:	
	static AvxLog* g_pLoggingServices;
	
private:
	AvxLog();
	
private:
	static char 		m_varArgsBuffer[1 + MAX_VARARGS_LEN];
	log4cpp::Appender*	m_pAppender;
	log4cpp::Layout*	m_pLayout;
	log4cpp::Category&	m_category;
};

} // namespace avxsynth

#endif // __AVX_LOG_H__
