#include "stdafx.h"

#include "zlua.h"


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//------------------------------------------------------------------------------------------
zlua::zlua() :
	m_lua_state(0)
{
}


//------------------------------------------------------------------------------------------
zlua::~zlua()
{
	close();
}


//------------------------------------------------------------------------------------------
void zlua::new_state()
{
	m_lua_state = luaL_newstate();
};


//------------------------------------------------------------------------------------------
void zlua::open_libs()
{
	dbg_assert(m_lua_state);
	luaL_openlibs(m_lua_state);
}


//------------------------------------------------------------------------------------------
void zlua::close()
{
	if(m_lua_state)
		lua_close(m_lua_state);
	m_lua_state = 0;
}


//------------------------------------------------------------------------------------------
bool zlua::load_script(const char *file_name)
{
	dbg_assert(m_lua_state);

	int ret = luaL_loadfile(m_lua_state, file_name);
	if(ret != 0)
	{
		_log_error();
		return false;
	}

    if(lua_pcall(m_lua_state, 0, 0, 0))
	{
		_log_error();
        return false;
	}

	return true;
}


//------------------------------------------------------------------------------------------
bool zlua::call_function(const char *func_name)
{
	lua_getglobal(m_lua_state, func_name);
	if(lua_pcall(m_lua_state, 0, 0, 0))
	{
		_log_error();
		return false;
	}

	return true;
}


//------------------------------------------------------------------------------------------
void zlua::_log_error()
{
	const char* err = lua_tostring(m_lua_state, -1);
	if(err)
	{
		zdebug::log()->LogImp(err);
		zdebug::log()->Log("!!!Lua error!!!\n");
		_ass(0);
	}

	lua_pop(m_lua_state, 1);
}

