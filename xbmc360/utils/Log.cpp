#include "Log.h"
#include <share.h>
#include "CriticalSection.h"
#include "SingleLock.h"
#include "Settings.h"
#include "utils\URIUtils.h"

FILE*       CLog::m_fd              = NULL;
int         CLog::m_repeatCount     = 0;
int         CLog::m_repeatLogLevel  = -1;
CStdString  CLog::m_repeatLine      = "";
int         CLog::m_logLevel        = LOG_LEVEL_DEBUG;

static CCriticalSection critSec;

static char levelNames[][8] =
{"DEBUG", "INFO", "NOTICE", "WARNING", "ERROR", "SEVERE", "FATAL", "NONE"};

#define LINE_ENDING "\n"

CLog::CLog()
{
}

CLog::~CLog()
{
}

void CLog::Close()
{
	CSingleLock waitLock(critSec);
	if(m_fd)
	{
		fclose(m_fd);
		m_fd = NULL;
	}
}

void CLog::Log(int loglevel, const char *format, ... )
{
	CSingleLock waitLock(critSec);

	if(m_logLevel > LOG_LEVEL_NORMAL ||
		(m_logLevel > LOG_LEVEL_NONE && loglevel >= LOGNOTICE))
	{
		if(!m_fd)
		{
			// We should only continue when the logfolder is set
			if(g_settings.m_logFolder.IsEmpty()) return;

			CStdString LogFile;
			URIUtils::AddFileToFolder(g_settings.m_logFolder, "xbmc.log", LogFile);
			m_fd = _fsopen(LogFile, "a+", _SH_DENYWR);
		}
      
		if(!m_fd)
			return;

		SYSTEMTIME time;
		GetLocalTime(&time);

		MEMORYSTATUS stat;
		GlobalMemoryStatus(&stat);

		CStdString strPrefix, strData;

		strPrefix.Format("%02.2d:%02.2d:%02.2d M:%9u %7s: ", time.wHour, time.wMinute, time.wSecond, stat.dwAvailPhys, levelNames[loglevel]);

		strData.reserve(16384);
		va_list va;
		va_start(va, format);
		strData.FormatV(format,va);
		va_end(va);

		if(m_repeatLogLevel == loglevel && m_repeatLine == strData)
		{
			m_repeatCount++;
			return;
		}
		else if(m_repeatCount)
		{
			CStdString strPrefix2, strData2;
			strPrefix.Format("%02.2d:%02.2d:%02.2d M:%9u %7s: ", time.wHour, time.wMinute, time.wSecond, stat.dwAvailPhys, levelNames[m_repeatLogLevel]);

			strData2.Format("Previous line repeats %d times." LINE_ENDING, m_repeatCount);
			fwrite(strPrefix2.c_str(), strPrefix2.size(), 1, m_fd);
			fwrite(strData2.c_str(), strData2.size(), 1, m_fd);
#ifdef _DEBUG
			OutputDebugString(strData2.c_str());
#endif
			m_repeatCount = 0;
		}
    
		m_repeatLine      = strData;
		m_repeatLogLevel  = loglevel;

		unsigned int length = 0;

		while(length != strData.length())
		{
			length = strData.length();
			strData.TrimRight(" ");
			strData.TrimRight('\n');
			strData.TrimRight("\r");
		}

		if(!length)
			return;

#ifdef _DEBUG
		OutputDebugString(strData.c_str());
		OutputDebugString("\n");
#endif

		// Fixup newline alignment, number of spaces should equal prefix length
		strData.Replace("\n", LINE_ENDING"                                            ");
		strData += LINE_ENDING;

		fwrite(strPrefix.c_str(), strPrefix.size(), 1, m_fd);
		fwrite(strData.c_str(), strData.size(), 1, m_fd);
		fflush(m_fd);
	}
#if defined(_DEBUG) || defined(PROFILE)
	else
	{
		// In debug mode dump everything to devstudio regardless of level
		CSingleLock waitLock(critSec);
		CStdString strData;
		strData.reserve(16384);

		va_list va;
		va_start(va, format);
		strData.FormatV(format, va);
		va_end(va);

		OutputDebugString(strData.c_str());
		if(strData.Right(1) != "\n")
			OutputDebugString("\n");
	}
#endif
}

void CLog::DebugLog(const char *format, ... )
{
#ifdef _DEBUG
	CSingleLock waitLock(critSec);

	CStdString strData;
	strData.reserve(16384);

	va_list va;
	va_start(va, format);
	strData.FormatV(format, va);
	va_end(va);

	OutputDebugString(strData.c_str());

	if(strData.Right(1) != "\n")
		OutputDebugString("\n");
#endif
}

void CLog::MemDump(BYTE *pData, int length)
{
	Log(LOGDEBUG, "MEM_DUMP: Dumping from %x", (unsigned int)pData);

	for(int i = 0; i < length; i+=16)
	{
		CStdString strLine;
		strLine.Format("MEM_DUMP: %04x ", i);
		BYTE *alpha = pData;
		
		for(int k=0; k < 4 && i + 4*k < length; k++)
		{
			for(int j=0; j < 4 && i + 4*k + j < length; j++)
			{
				CStdString strFormat;
				strFormat.Format(" %02x", *pData++);
				strLine += strFormat;
			}
			strLine += " ";
		}

		// Pad with spaces
		while(strLine.size() < 13*4 + 16)
			strLine += " ";

		for(int j=0; j < 16 && i + j < length; j++)
		{
			CStdString strFormat;

			if(*alpha > 31 && *alpha < 128)
				strLine += *alpha;
			else
				strLine += '.';

			alpha++;
		}
		Log(LOGDEBUG, "%s", strLine.c_str());
	}
}

void CLog::SetLogLevel(int level)
{
	CSingleLock waitLock(critSec);
	m_logLevel = level;
	CLog::Log(LOGNOTICE, "Log level changed to %d", m_logLevel);
}

int CLog::GetLogLevel()
{
	return m_logLevel;
}