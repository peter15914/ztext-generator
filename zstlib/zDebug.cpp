#include "stdafx.h"

#include "zDebug.h"

#include <zstlib/utl/iLog.h>
#include "zStr.h"

#include <assert.h>


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


namespace zdebug
{
	iLog *g_log = 0;
	bool g_debugbreak = true;
	bool g_was_error2 = false;
}


//------------------------------------------------------------------------------------------
//void zdebug::DebugAssertW(const wchar_t *_sExp, const wchar_t *_sFile, unsigned _iLine )
//{
//	if(zdebug::g_log)
//		zdebug::g_log->Error(zstr::wfmt(L"assert(%s) in %s, line %d", _sExp, _sFile, _iLine));
//}


//------------------------------------------------------------------------------------------
void zdebug::ReleaseAssert(const wchar_t *_sExp, const wchar_t *_sFile, unsigned _iLine )
{
	if(zdebug::g_log)
		zdebug::g_log->Error(zstr::wfmt(L"!!!assert(%s) in %s, line %d", _sExp, _sFile, _iLine));

	g_was_error2 = true;
}


//------------------------------------------------------------------------------------------
void zdebug::ReleaseAssertText(const char *text, const wchar_t *_sFile, unsigned _iLine )
{
	using namespace zstr;
	if(zdebug::g_log)
	{
		zdebug::g_log->Log(w_to_s(wfmt(L"!!!warning in %s, line %d", _sFile, _iLine)));
		zdebug::g_log->Error(fmt("%s", text));
	}

	g_was_error2 = true;
}

//------------------------------------------------------------------------------------------
void zdebug::ReleaseAssertText(const std::string &text, const wchar_t *_sFile, unsigned _iLine )
{
	ReleaseAssertText(text.c_str(), _sFile, _iLine );
}


//------------------------------------------------------------------------------------------
void zdebug::set_logger(iLog *log)
{
	g_log = log;
}


//------------------------------------------------------------------------------------------
iLog *zdebug::log()
{
	return g_log;
}


//------------------------------------------------------------------------------------------
//делать ли debugbreak
void zdebug::set_debugbreak(bool debugbreak)
{
	g_debugbreak = debugbreak;
}


//------------------------------------------------------------------------------------------
void zdebug::Log(const std::string &str)
{
	if(zdebug::g_log)
		g_log->Log(str);
	else
		(_ass(0));
}


//------------------------------------------------------------------------------------------
void zdebug::LogImp(const std::string &str)
{
	if(zdebug::g_log)
		g_log->LogImp(str);
	else
		(_ass(0));
}


//------------------------------------------------------------------------------------------
void zdebug::Log(boost::format &statement)
{
	Log(statement.str());
}


//------------------------------------------------------------------------------------------
void zdebug::LogImp(boost::format &statement)
{
	LogImp(statement.str());
}

