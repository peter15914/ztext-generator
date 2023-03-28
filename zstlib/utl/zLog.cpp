#include "stdafx.h"

#include "zLog.h"

#include <iostream>
#include <Windows.h>		//для timeGetTime
#include <Mmsystem.h>		//для timeGetTime

#include <zstlib/zstr.h>


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "winmm.lib")

//------------------------------------------------------------------------------
zLog::zLog() :
	m_bWasError(false),
	m_bWriteToCout(false),
	m_bWriteToTrace(false),
	m_dPrev(0),
	m_pLogSubscriber(0)
{
	m_dCreateTime = timeGetTime();
	m_dPrev = m_dCreateTime;
}


//------------------------------------------------------------------------------
zLog::~zLog()
{
	int sec = (timeGetTime() - m_dCreateTime) / 1000;
	Log(zstr::fmt("application work time: %d:%d", sec / 60, sec % 60));
	m_File.close();
}


//------------------------------------------------------------------------------
void zLog::Open(const std::wstring &_wsFileName)
{
	m_wsFileName = _wsFileName;
	m_File.open(_wsFileName.c_str());
	_ass(m_File);
	if(!m_File)
		exit(1);
}


//------------------------------------------------------------------------------
void zLog::_LogEx(const std::string &str, bool bFlush)
{
	if(m_File.is_open())
		m_File << str << "\n";

	//
	if(m_bWriteToCout)
		std::cout << str << "\n";

#ifdef _DEBUG
	if(m_bWriteToTrace)
	{
		OutputDebugStringA(str.c_str());
		OutputDebugStringA("\n");
	}
#endif
	//
	if(bFlush && m_File)
		m_File.flush();

	//
	if(m_pLogSubscriber)
		m_pLogSubscriber->Log(str);
}


//------------------------------------------------------------------------------
void zLog::Log(const std::string &str)
{
#ifdef _DEBUG
	_LogEx(str, true);
#else
	_LogEx(str, false);
#endif
}


//------------------------------------------------------------------------------
void zLog::LogImp(const std::string &str)
{
	_LogEx(str, true);
}


//------------------------------------------------------------------------------
void zLog::Error(const std::string &str)
{
	LogImp(str);
	m_bWasError = true;
}


//------------------------------------------------------------------------------
void zLog::Error(const std::wstring &str)
{
	LogImp(zstr::w_to_s(str));
	m_bWasError = true;
}


//------------------------------------------------------------------------------
void zLog::Flush()
{
	if(m_File)
		m_File.flush();
}


//------------------------------------------------------------------------------
void zLog::ResetTime()
{
	m_dPrev = timeGetTime();
}


//------------------------------------------------------------------------------
//выдает, сколько времени прошло с предыдущего вызова функции
std::string zLog::GetLogTimeSpend(const char *s/* = 0*/)
{
	DWORD dCur = timeGetTime();
	DWORD dd = dCur - m_dPrev;
	m_dPrev = dCur;

	std::string ret = zstr::fmt("%lu.%03lu", dd / 1000, dd % 1000);

	if(!s)
		ret = "time spend: " + ret;
	else
		ret = "time spend (" + std::string(s) + "): " + ret;


	return ret;
}


//------------------------------------------------------------------------------
void zLog::SetLogSubscriber(iLogSubscriber *pLogSubscriber)
{
	m_pLogSubscriber = pLogSubscriber;
}

