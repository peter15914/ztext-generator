#pragma once

/*/----------------------------------------------------------------------------
	@desc
		debug routines


	2008May15 created. zm.
-----------------------------------------------------------------------------*/

class iLog;

//------------------------------------------------------------------------------------------
namespace zdebug
{
	void set_logger(iLog *log);
	iLog *log();

	//делать ли debugbreak
	void set_debugbreak(bool debugbreak);

	//void DebugAssertW(const wchar_t *_sExp, const wchar_t *_sFile, unsigned _iLine);
	void ReleaseAssert(const wchar_t *_sExp, const wchar_t *_sFile, unsigned _iLine);
	void ReleaseAssertText(const char *text, const wchar_t *_sFile, unsigned _iLine);
	void ReleaseAssertText(const std::string &text, const wchar_t *_sFile, unsigned _iLine);

	void Log(const std::string &str);
	void LogImp(const std::string &str);

	void Log(boost::format &statement);
	void LogImp(boost::format &statement);

	extern bool g_debugbreak;
	extern bool g_was_error2;
};



//------------------------------------------------------------------------------------------
#define rel_assert(exp)														\
	(void)( (exp) || (zdebug::ReleaseAssert(_CRT_WIDE(#exp), _CRT_WIDE(__FILE__), __LINE__), 0) ||	\
		!zdebug::g_debugbreak || !IsDebuggerPresent() || (__debugbreak(), 0))

//------------------------------------------------------------------------------------------
#define rel_warn(text)														\
	(void)( (zdebug::ReleaseAssertText(text, _CRT_WIDE(__FILE__), __LINE__), 0))

//------------------------------------------------------------------------------------------
//то же самое, что и rel_assert, только покороче название
#define _ass(exp) rel_assert(exp)

//то же самое, что и rel_assert, только покороче название
#define _warn(exp) rel_warn(exp)

//------------------------------------------------------------------------------------------
//dbg_assert работает только в дебаговой версии - для критичных по скорости мест
#ifdef _DEBUG

	#define dbg_assert(exp) rel_assert(exp)

#else
    //
	#define dbg_assert(exp)			((void)0)
	//
#endif
//------------------------------------------------------------------------------------------

