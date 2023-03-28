#pragma once

/*/----------------------------------------------------------------------------
	@namespace iLog
	@desc
		интерфейс для zLog
	
	2011Oct19 created. zm.
-----------------------------------------------------------------------------*/

class iLogSubscriber;

//------------------------------------------------------------------------------
class iLog
{
public:
	virtual ~iLog() {};

	virtual void Open(const std::wstring &_wsFileName) = 0;
	virtual void Log(const std::string &str) = 0;
	virtual void LogImp(const std::string &str) = 0;
	virtual void Error(const std::string &str) = 0;
	virtual void Error(const std::wstring &str) = 0;
	virtual void Flush() = 0;
	//
	virtual bool WasError() = 0;
	virtual void ResetError() = 0;
	//
	virtual void SetWriteToCout(bool write_copy) = 0;
	virtual void SetWriteToTrace(bool write_copy) = 0;
	//
	virtual void ResetTime() = 0;
	virtual std::string GetLogTimeSpend(const char *s = 0) = 0;
	//
	virtual void SetLogSubscriber(iLogSubscriber *pLogSubscriber) = 0;
};


//------------------------------------------------------------------------------
class iLogSubscriber
{
public:
	virtual void Log(const std::string &str) = 0;
};
