#pragma once

/*/----------------------------------------------------------------------------
	@namespace zLog
	@desc
		лёгкий, ненавязчивый класс для работы с логами

	2008May22 created. zm
-----------------------------------------------------------------------------*/

#include "iLog.h"

#include <fstream>

class iLogSubscriber;

//------------------------------------------------------------------------------
class zLog : public iLog
{
	std::wstring m_wsFileName;
	std::ofstream m_File;
	//
	bool m_bWasError;
	//
	bool m_bWriteToCout;
	bool m_bWriteToTrace;

	DWORD m_dCreateTime;
	DWORD m_dPrev;

	iLogSubscriber *m_pLogSubscriber;

public:
	zLog();
	virtual ~zLog();
	//
	virtual void Open(const std::wstring &_wsFileName);
	virtual void Log(const std::string &str);
	virtual void LogImp(const std::string &str);
	virtual void Error(const std::string &str);
	virtual void Error(const std::wstring &str);
	virtual void Flush();
	//
	virtual bool WasError() { return m_bWasError; }
	virtual void ResetError() { m_bWasError = false; }
	//
	virtual void SetWriteToCout(bool write_copy) { m_bWriteToCout = write_copy; };
	virtual void SetWriteToTrace(bool write_copy) { m_bWriteToTrace = write_copy; };

	//выдает, сколько времени прошло с предыдущего вызова функции
	std::string GetLogTimeSpend(const char *s = 0);
	void ResetTime();

	virtual void SetLogSubscriber(iLogSubscriber *pLogSubscriber);

private:
	void _LogEx(const std::string &str, bool bFlush);
};

