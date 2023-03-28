#include "stdafx.h"

#include "zparser.h"

#include <zstlib/zstr.h>
#include <zstlib/utl/zLog.h>

using namespace std;


//------------------------------------------------------------------------------------------
DWORD g_t0 = 0;

namespace
{
	const wchar_t *LOG_FILE_NAME = L"__error.log";
	const wchar_t *LOG_FILE_NAME2 = L"__error2.log";
};


//------------------------------------------------------------------------------------------
zParserApp::zParserApp() :
	m_gen_procs(0)
{
}


//------------------------------------------------------------------------------------------
zParserApp::~zParserApp()
{
	delete m_gen_procs;
	m_gen_procs = 0;
}


//------------------------------------------------------------------------------------------
zParserApp &zParserApp::inst()
{
	static zParserApp app;
	return app;
}


//------------------------------------------------------------------------------------------
//void _main_seh()
//{
//	__try
//	{
//		_main();
//	}
//	__finally
//	{
//		_fatal_error_handling();
//	}
//}


//------------------------------------------------------------------------------------------
int main(int argc, _TCHAR* argv[])
{
	g_t0 = timeGetTime();

	zutl::MakeBackupsEx(LOG_FILE_NAME, 50, L"_logs", 5);
	zLog log;
	log.SetWriteToCout(true);
	log.Open(LOG_FILE_NAME);
	zdebug::set_logger(&log);
	zdebug::log()->Log("--------");

//---------------------------------------
	////это функция для чтения указанной книги (argv[1]) в указанный файл (argv[2])
	//if(argc >= 3)
	//{
	//	std::string name = argv[1];
	//	std::string sResFile = argv[2];
	//	output_book("fb2/" + name, zstr::s_to_w(sResFile));
	//}
//---------------------------------------
if(0)
{
	zParserApp::inst().main_proc();
}

//---------------------------------------
if(1)
{
	//запуск lua-скрипта
	zlua &lua = zParserApp::inst().lua();
	lua.new_state();
	lua.open_libs();

	zParserApp::inst().bind_scripts();

	bool ret = lua.load_script("main.lua");
	rel_assert(ret);

	if(ret)
	{
		lua.call_function("main");
	}

	lua.close();
}
//---------------------------------------

	//
	if(zdebug::log()->WasError())
		zdebug::log()->Log("!!!!!!!!!!!!!!!!WAS  ERROR!!!!!!!!!!!!!");

	//Beep( 750, 500 );
	//MessageBox(0, "job finished", "job finished", MB_OK);

	zdebug::log()->Log("finishing application");
	return 0;
}
