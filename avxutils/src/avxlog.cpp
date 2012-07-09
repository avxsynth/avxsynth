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

#include "avxlog.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <memory.h>
#include <limits.h>

namespace avxsynth
{
    
AvxLog* AvxLog::g_pLoggingServices = new AvxLog();
char 	AvxLog::m_varArgsBuffer[1 + MAX_VARARGS_LEN];

void AvxLog::Debug(const char* pStrModule, const char* pStrFormat, ...)
{
	// Notice: 
	//
	// the default compiler flags option as returned by `log4cpp-config --cflags`
	// has the -DNDEBUG set, which effectively blocks Debug messages. 
	//
	if(NULL == g_pLoggingServices)
		return; // throw AVSException
		
	memset(AvxLog::m_varArgsBuffer, 0, (1 + MAX_VARARGS_LEN)*sizeof(char));
	sprintf(AvxLog::m_varArgsBuffer, "%s,Type=Debug,Message=", pStrModule);
	
	char* pTemp = AvxLog::m_varArgsBuffer + strlen(AvxLog::m_varArgsBuffer);
	va_list args;
	va_start (args, pStrFormat);
	vsprintf (pTemp, pStrFormat, args);
	va_end (args);	
		
	g_pLoggingServices->m_category.debug(AvxLog::m_varArgsBuffer);
}

void AvxLog::Info(const char* pStrModule, const char* pStrFormat, ...)
{
	if(NULL == g_pLoggingServices)
		return; // throw AVSException
		
	memset(AvxLog::m_varArgsBuffer, 0, (1 + MAX_VARARGS_LEN)*sizeof(char));
	sprintf(AvxLog::m_varArgsBuffer, "%s,Type=Info,Message=", pStrModule);
	
	char* pTemp = AvxLog::m_varArgsBuffer + strlen(AvxLog::m_varArgsBuffer);
	va_list args;
	va_start (args, pStrFormat);
	vsprintf (pTemp, pStrFormat, args);
	va_end (args);	

	g_pLoggingServices->m_category.info(AvxLog::m_varArgsBuffer);
}

void AvxLog::Notice(const char* pStrModule, const char* pStrFormat, ...)
{
	if(NULL == g_pLoggingServices)
		return; // throw AVSException
		
	memset(AvxLog::m_varArgsBuffer, 0, (1 + MAX_VARARGS_LEN)*sizeof(char));
	sprintf(AvxLog::m_varArgsBuffer, "%s,Type=Notice,Message=", pStrModule);
	
	char* pTemp = AvxLog::m_varArgsBuffer + strlen(AvxLog::m_varArgsBuffer);
	va_list args;
	va_start (args, pStrFormat);
	vsprintf (pTemp, pStrFormat, args);
	va_end (args);	

	g_pLoggingServices->m_category.notice(AvxLog::m_varArgsBuffer);
}

void AvxLog::Warn(const char* pStrModule, const char* pStrFormat, ...)
{
	if(NULL == g_pLoggingServices)
		return; // throw AVSException
	
	memset(AvxLog::m_varArgsBuffer, 0, (1 + MAX_VARARGS_LEN)*sizeof(char));
	sprintf(AvxLog::m_varArgsBuffer, "%s,Type=Warn,Message=", pStrModule);
	
	char* pTemp = AvxLog::m_varArgsBuffer + strlen(AvxLog::m_varArgsBuffer);
	va_list args;
	va_start (args, pStrFormat);
	vsprintf (pTemp, pStrFormat, args);
	va_end (args);	

	g_pLoggingServices->m_category.warn(AvxLog::m_varArgsBuffer);
}

void AvxLog::Error(const char* pStrModule, const char* pStrFormat, ...)
{
	if(NULL == g_pLoggingServices)
		return; // throw AVSException
	
	memset(AvxLog::m_varArgsBuffer, 0, (1 + MAX_VARARGS_LEN)*sizeof(char));
	sprintf(AvxLog::m_varArgsBuffer, "%s,Type=Error,Message=", pStrModule);
	
	char* pTemp = AvxLog::m_varArgsBuffer + strlen(AvxLog::m_varArgsBuffer);
	va_list args;
	va_start (args, pStrFormat);
	vsprintf (pTemp, pStrFormat, args);
	va_end (args);	

	g_pLoggingServices->m_category.error(AvxLog::m_varArgsBuffer);
}

void AvxLog::Crit(const char* pStrModule, const char* pStrFormat, ...)
{
	if(NULL == g_pLoggingServices)
		return; // throw AVSException
		
	memset(AvxLog::m_varArgsBuffer, 0, (1 + MAX_VARARGS_LEN)*sizeof(char));
	sprintf(AvxLog::m_varArgsBuffer, "%s,Type=Crit,Message=", pStrModule);
	
	char* pTemp = AvxLog::m_varArgsBuffer + strlen(AvxLog::m_varArgsBuffer);
	va_list args;
	va_start (args, pStrFormat);
	vsprintf (pTemp, pStrFormat, args);
	va_end (args);	

	g_pLoggingServices->m_category.crit(AvxLog::m_varArgsBuffer);
}

void AvxLog::Alert(const char* pStrModule, const char* pStrFormat, ...)
{
	if(NULL == g_pLoggingServices)
		return; // throw AVSException
	
	memset(AvxLog::m_varArgsBuffer, 0, (1 + MAX_VARARGS_LEN)*sizeof(char));
	sprintf(AvxLog::m_varArgsBuffer, "%s,Type=Alert,Message=", pStrModule);
	
	char* pTemp = AvxLog::m_varArgsBuffer + strlen(AvxLog::m_varArgsBuffer);
	va_list args;
	va_start (args, pStrFormat);
	vsprintf (pTemp, pStrFormat, args);
	va_end (args);	

	g_pLoggingServices->m_category.alert(AvxLog::m_varArgsBuffer);		
}

void AvxLog::Fatal(const char* pStrModule, const char* pStrFormat, ...)
{
	if(NULL == g_pLoggingServices)
		return; // throw AVSException
	
	memset(AvxLog::m_varArgsBuffer, 0, (1 + MAX_VARARGS_LEN)*sizeof(char));
	sprintf(AvxLog::m_varArgsBuffer, "%s,Type=Fatal,Message=", pStrModule);
	
	char* pTemp = AvxLog::m_varArgsBuffer + strlen(AvxLog::m_varArgsBuffer);
	va_list args;
	va_start (args, pStrFormat);
	vsprintf (pTemp, pStrFormat, args);
	va_end (args);	

	g_pLoggingServices->m_category.fatal(AvxLog::m_varArgsBuffer);
}	
	
const char* AvxLog::DetermineLoggingFolderPath(void)
{
    //
    // The default location for log files is $HOME/.avxsynth/logs
    //
    char* pStrHomeFolderPath = getenv("HOME");
    if(NULL == pStrHomeFolderPath)
    {
        fprintf(stderr, "Failed retrieving the value of $HOME env variable\n");
        return NULL;
    }
    
    std::string strAvxSynthFolder   = std::string(pStrHomeFolderPath) + std::string("/.avxsynth");
    std::string strLoggingConfFile  = strAvxSynthFolder + std::string("/avxsynthlog.conf");
    std::string strDefaultLogPath   = strAvxSynthFolder + std::string("/log");
    
    struct stat st;
    bool bAvxSynthFolderExists      = (0 == stat(strAvxSynthFolder.c_str(), &st));
    bool bLoggingConfFileExists     = (0 == stat(strLoggingConfFile.c_str(), &st));
    bool bDefaultLogFolderExists    = (0 == stat(strDefaultLogPath.c_str(), &st));
    
    if(bAvxSynthFolderExists)
    {
        if(bLoggingConfFileExists)
        {
            //
            // We will read the file $HOME/.avxsynth/avxsynthlogs.conf and
            // extract from there the information about the logging folder path
            //
            FILE* fp = fopen(strLoggingConfFile.c_str(), "r");
            if(NULL == fp)
            {
                fprintf(stderr, "Failed opening %s for reading\n", strLoggingConfFile.c_str());
                return NULL;
            }
            
            std::string strSpecifiedLogPath;
            char strLine[PATH_MAX];
            memset(strLine, 0, PATH_MAX*sizeof(char));
            while(1)
            {
                if(NULL == fgets(strLine, PATH_MAX, fp))
                    break;
                
                if(strLine == strstr(strLine, "LOG_PATH="))
                {
                    strSpecifiedLogPath = strLine;
                    strSpecifiedLogPath.replace(0, strlen("LOG_PATH="), "");
                    size_t nNewlinePos = strSpecifiedLogPath.find("\n");
                    if(std::string::npos != nNewlinePos)
                        strSpecifiedLogPath.replace(nNewlinePos, 1, "");
                    break;
                }
            }
            fclose(fp);
            fp = NULL;
            
            if(strSpecifiedLogPath.empty())
            {
                fprintf(stderr, "No valid avxsynth log path found in %s\n", strLoggingConfFile.c_str());
                return NULL;
            }
            else
            {
                bool bSpecifiedLogPathExists = (0 == stat(strSpecifiedLogPath.c_str(), &st));
                if(false == bSpecifiedLogPathExists)
                {
                    if(mkdir(strSpecifiedLogPath.c_str(), 0777))
                    {
                        fprintf(stderr, "Failed creating non-existing folder %s (specified in %s)\n", 
                                            strSpecifiedLogPath.c_str(), strLoggingConfFile.c_str());
                        return NULL;
                    }
                }
                return strSpecifiedLogPath.c_str();
            }
        }
        else
        {
            //
            // $HOME/.avxsynth/avxsynthconf.log does not exist
            // 
            FILE* fp = fopen(strLoggingConfFile.c_str(), "w");
            if(NULL == fp)
            {
                fprintf(stderr, "Failed creating non-existent %s\n", strLoggingConfFile.c_str());
                return NULL;
            }
            
            if(false == bDefaultLogFolderExists)
            {
                if(mkdir(strDefaultLogPath.c_str(), 0777))
                {
                    fclose(fp);
                    fp = NULL;
                    fprintf(stderr, "Failed creating non-existent default log folder %s\n", strDefaultLogPath.c_str());
                    return NULL;
                }
            }
            fprintf(fp, "# Syntax: LOG_PATH=<log folder path> // do not use double quotes\n");
            fprintf(fp, "LOG_PATH=%s\n", strDefaultLogPath.c_str());
            fclose(fp);
            fp = NULL;
            return strDefaultLogPath.c_str();
        }
    }
    else
    {
        //
        // Let's create defaults all the way
        // 
        if(mkdir(strAvxSynthFolder.c_str(), 0777))
        {
            fprintf(stderr, "Failed creating non-existent %s folder\n", strAvxSynthFolder.c_str());
            return NULL;
        }
        if(mkdir(strDefaultLogPath.c_str(), 0777))
        {
            fprintf(stderr, "Failed creating non-existent default logging folder %s\n", strDefaultLogPath.c_str());
            return NULL;
        }
        FILE* fp = fopen(strLoggingConfFile.c_str(), "w");
        if(NULL == fp)
        {
            fprintf(stderr, "Failed creating non-existent %s file\n", strLoggingConfFile.c_str());
            return NULL;
        }
        fprintf(fp, "# Syntax: LOG_PATH=<log folder path> // do not use double quotes\n");
        fprintf(fp, "LOG_PATH=%s\n", strDefaultLogPath.c_str());
        fclose(fp);
        fp = NULL;
        return strDefaultLogPath.c_str();
    }
    return NULL;
}

AvxLog::AvxLog()
	: m_pOstream(NULL)
	, m_pAppender(NULL)
	, m_pLayout(NULL)
	, m_category(log4cpp::Category::getInstance("Category"))
{
    const char* pStrLogFolder = DetermineLoggingFolderPath();
    if(pStrLogFolder)
    {
        char strLogFilename[PATH_MAX];
        memset(strLogFilename, 0, PATH_MAX*sizeof(char));
        
        pid_t currentPID = getpid();
        
        sprintf(strLogFilename, "%s/logAvxsynth_pid_%08d.txt", pStrLogFolder, currentPID);
        m_fb.open(strLogFilename, std::ios::out);
        m_pOstream = new std::ostream(&m_fb);
    }
    else
        m_pOstream = &std::cerr;
    
	m_pAppender = new log4cpp::OstreamAppender("OstreamAppender", m_pOstream);
	m_pLayout   = new log4cpp::SimpleLayout();

	m_pAppender->setLayout(m_pLayout);
	m_category.setAppender(m_pAppender);
    m_category.setPriority(log4cpp::Priority::INFO);
};

AvxLog::~AvxLog()
{
    if(m_pOstream && (&std::cerr != m_pOstream))
    {
        m_fb.close();
        delete m_pOstream;
        m_pOstream = NULL;
    }
    m_category.removeAllAppenders();
    m_category.shutdown();
}

void AvxLog::TerminateLogging()
{
    if(AvxLog::g_pLoggingServices)
    {
        delete AvxLog::g_pLoggingServices;
        AvxLog::g_pLoggingServices = NULL;
    }
}

} // namespace avxsynth
