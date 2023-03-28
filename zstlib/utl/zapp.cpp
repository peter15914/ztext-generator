#include "stdafx.h"

#include "zapp.h"

#include <zstlib/zutl.h>

#include <conio.h>	//для _getch()


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;


zApp* zApp::m_inst = 0;

//------------------------------------------------------------------------------------------
zApp::zApp() :
	m_call_lua(true),
	m_use_debug_break(true),
	m_console_app(true),
	m_exit_instance_called(false)
{
	m_error_log_name = L"log";
	zApp::m_inst = this;

	m_argc = 0;
	m_argv = 0;
}


//------------------------------------------------------------------------------------------
zApp::~zApp()
{
	_ass(m_exit_instance_called);
}


//------------------------------------------------------------------------------------------
int zApp::main_start()
{
	zutl::check_debug_heap();

	wstring log_name = m_error_log_name + zstr::s_to_w(zutl::GetDateTimeStrFull()) + L".log";
	m_log.Open(log_name);
	zdebug::set_logger(&m_log);

	zdebug::log()->Log("--------");
	zdebug::log()->Log(zstr::fmt("--build time--<%s %s>----", __DATE__, __TIME__));
	zdebug::log()->Log("--------\n");

	zdebug::set_debugbreak(m_use_debug_break);

	return 0;
}


//------------------------------------------------------------------------------------------
int zApp::main_process()
{
	if(!m_call_lua)
	{
		some_func();
	}
	else
	{
#ifndef ZZZ_NO_LUA
		//запуск lua-скрипта
		m_lua.new_state();
		m_lua.open_libs();

		this->bind_scripts();

		bool ret = m_lua.load_script("main.lua");
		rel_assert(ret);

		if(ret)
		{
			m_lua.call_function("main");
		}

		m_lua.close();
#else
	_ass(0);	//если не исользуете lua, то устанавливайте !m_call_lua
#endif
	}

	return 0;
}


//------------------------------------------------------------------------------------------
int zApp::main_finish()
{
	if(zdebug::log()->WasError())
		zdebug::log()->LogImp("!!!!!!!!!!!!!!!!WAS  ERROR!!!!!!!!!!!!!");

	zdebug::log()->LogImp("\n--------\nfinishing application");

	exit_instance();

	return 0;
}


//------------------------------------------------------------------------------------------
//функция вызывается ровно один раз из _tmain()
int zApp::main(int argc, _TCHAR* argv[])
{
	if(!m_console_app)
		return (_ass(0), 1);

	m_argc = argc;
	m_argv = argv;

	setlocale(LC_ALL, "russian_russia.1251");

	m_log.SetWriteToCout(true);

	main_start();
	main_process();
	main_finish();

	zdebug::Log("Press any key");
	_getch();

	return 0;
}


//------------------------------------------------------------------------------------------
//для удобства что-то иногда в ней реализовываем
void zApp::some_func()
{
}

#ifndef ZZZ_NO_LUA
//------------------------------------------------------------------------------------------
static int lua_Log(lua_State *L)
{
	const char *s = lua_tostring(L, 1);

	if(zdebug::log())
		zdebug::log()->Log(s);
	else
		(_ass(0));

	return 0;
}


//------------------------------------------------------------------------------------------
static int lua_LogImp(lua_State *L)
{
	const char *s = lua_tostring(L, 1);

	if(zdebug::log())
		zdebug::log()->LogImp(s);
	else
		(_ass(0));

	return 0;
}


//------------------------------------------------------------------------------------------
static int lua_Sleep(lua_State *L)
{
	int i1 = (int)lua_tointeger(L, 1);
	Sleep(i1);
	return 0;
}

//------------------------------------------------------------------------------------------
static int _copy_file(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);
	const char *s2 = lua_tostring(L, 2);
	
	boost::filesystem::path p2(s2);
	boost::filesystem::create_directories(p2.parent_path());

	try
	{
		boost::filesystem::copy_file(s1, s2);
	}
	catch (...)
	{
		_ass(0);
	}

	return 0;
}


//------------------------------------------------------------------------------------------
static bool _copyDir(
    boost::filesystem::path const & source,
    boost::filesystem::path const & destination
)
{
    namespace fs = boost::filesystem;
    try
    {
        // Check whether the function call is valid
        if(!fs::exists(source) || !fs::is_directory(source))
            return (_ass(0), false);
        if(fs::exists(destination))
            return (_ass(0), false);

        // Create the destination directory
        if(!fs::create_directories(destination))
            return (_ass(0), false);
    }
    catch(fs::filesystem_error const & e)
    {
		zdebug::LogImp(e.what());
        return (_ass(0), false);
    }

    // Iterate through the source directory
    for(
        fs::directory_iterator file(source);
        file != fs::directory_iterator(); ++file
    )
    {
        try
        {
            fs::path current(file->path());
            if(fs::is_directory(current))
            {
                // Found directory: Recursion
                if(
                    !_copyDir(
                        current,
                        destination / current.filename()
                    )
                )
                {
                    return false;
                }
            }
            else
            {
                // Found file: Copy
                fs::copy_file(
                    current,
                    destination / current.filename()
                );
            }
        }
        catch(fs::filesystem_error const & e)
        {
			zdebug::LogImp(e.what());
			return (_ass(0), false);
		}
    }
    return true;
}

//------------------------------------------------------------------------------------------
static int _copy_dir(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);
	const char *s2 = lua_tostring(L, 2);

	_copyDir(s1, s2);

	return 0;
}


//------------------------------------------------------------------------------------------
static int _dirfile_exists(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);

	bool ret = boost::filesystem::exists(s1);
	lua_pushboolean(L, ret);

	return 1;
}


//------------------------------------------------------------------------------------------
static int _to_utf_8(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);

	std::string s = s1;
	s = zstr::ansi_to_utf8(s);
	lua_pushstring(L, s.c_str());

	return 1;
}


//------------------------------------------------------------------------------------------
void lua_stacktrace(lua_State* L)
{
    lua_Debug entry;
    int depth = 0; 

	zdebug::LogImp("----------------------lua_stacktrace");

    while (lua_getstack(L, depth, &entry))
	{
        int status = lua_getinfo(L, "Sln", &entry);
		assert(status);

		zdebug::LogImp(boost::format("%s(%d): %s") % entry.short_src % entry.currentline % (entry.name ? entry.name : "?"));
        depth++;
    }

	zdebug::LogImp("----------------------end lua_stacktrace");
}



//------------------------------------------------------------------------------------------
static int _z_assert(lua_State *L)
{
	int i1 = (int)lua_tointeger(L, 1);

	lua_stacktrace(L);
	_ass(i1);

	return 0;
}


//------------------------------------------------------------------------------------------
static int z_replace_substr_in_file(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);
	const char *s2 = lua_tostring(L, 2);
	const char *s3 = lua_tostring(L, 3);

	zutl::replace_substr_in_file(zstr::s_to_w(s1).c_str(), s2, s3);

	return 0;
}


//------------------------------------------------------------------------------------------
static int z_replace_substr_in_file_backup(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);
	const char *s2 = lua_tostring(L, 2);
	const char *s3 = lua_tostring(L, 3);
	int i4 = (int)lua_tointeger(L, 4);

	zutl::replace_substr_in_file(zstr::s_to_w(s1).c_str(), s2, s3, i4);

	return 0;
}


//------------------------------------------------------------------------------------------
static int z_replace_substr_in_file_from_file(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);
	const char *s2 = lua_tostring(L, 2);
	const char *s3 = lua_tostring(L, 3);
	int i4 = (int)lua_tointeger(L, 4);

	zutl::replace_substr_in_file_from_file(zstr::s_to_w(s1).c_str(), s2, zstr::s_to_w(s3).c_str(), i4);

	return 0;
}


//------------------------------------------------------------------------------------------
//биндаем функции для lua
void zApp::bind_scripts()
{
	lua_pushcfunction(m_lua.state(), lua_Log);
    lua_setglobal(m_lua.state(), "z_Log");

    lua_pushcfunction(m_lua.state(), lua_LogImp);
    lua_setglobal(m_lua.state(), "z_LogImp");

    lua_pushcfunction(m_lua.state(), lua_Sleep);
    lua_setglobal(m_lua.state(), "z_Sleep");

    lua_pushcfunction(m_lua.state(), _copy_file);
    lua_setglobal(m_lua.state(), "z_CopyFile");

    lua_pushcfunction(m_lua.state(), _copy_dir);
    lua_setglobal(m_lua.state(), "z_CopyDir");

    lua_pushcfunction(m_lua.state(), _dirfile_exists);
    lua_setglobal(m_lua.state(), "z_DirFileExists");

    lua_pushcfunction(m_lua.state(), _to_utf_8);
    lua_setglobal(m_lua.state(), "z_ToUtf8");

    lua_pushcfunction(m_lua.state(), _z_assert);
    lua_setglobal(m_lua.state(), "z_ass");

    lua_pushcfunction(m_lua.state(), z_replace_substr_in_file);
    lua_setglobal(m_lua.state(), "z_ReplaceSubstrInFile");

    lua_pushcfunction(m_lua.state(), z_replace_substr_in_file_backup);
    lua_setglobal(m_lua.state(), "z_ReplaceSubstrInFileB");

    lua_pushcfunction(m_lua.state(), z_replace_substr_in_file_from_file);
    lua_setglobal(m_lua.state(), "z_ReplaceSubstrInFileFromFile");
}
#endif	//#ifndef ZZZ_NO_LUA

//------------------------------------------------------------------------------------------
//должна вызываться в конце работы программы (чтобы ничего не делать в деструкторах статических объектов)
void zApp::exit_instance()
{
	m_exit_instance_called = true;
}
