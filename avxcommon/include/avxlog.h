// Copyright (c) 2012 Anne Aaron, David Ronca, Milan Stevanovic, Pradip Gajjar.  All rights reserved.
// avxanne@gmail.com, videophool@hotmail.com, avxsynth@gmail.com, pradip.gajjar@gmail.com
// http://www.avxsynth.org


// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
// Linking Avisynth statically or dynamically with other modules is making a
// combined work based on Avisynth.  Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Avisynth give you
// permission to link Avisynth with independent modules that communicate with
// Avisynth solely through the interfaces defined in avisynth.h, regardless of the license
// terms of these independent modules, and to copy and distribute the
// resulting combined work under terms of your choice, provided that
// every copy of the combined work is accompanied by a complete copy of
// the source code of Avisynth (the version of Avisynth used to produce the
// combined work), being distributed under the terms of the GNU General
// Public License plus this exception.  An independent module is a module
// which is not derived from or based on Avisynth, such as 3rd-party filters,
// import and export plugins, or graphical user interfaces.auto

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
