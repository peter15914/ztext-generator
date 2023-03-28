#pragma once

/*/----------------------------------------------------------------------------
	@zlua
	@desc
		zlua - небольшая обертка для работы с lua
		просто оборачивает некоторые функции, совсем не отменяет работу с функциями lua


	2012Feb16 created. zm
-----------------------------------------------------------------------------*/

extern "C"
{
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}


//------------------------------------------------------------------------------------------
#define ZLUA_BIND_FUNC(func_name) \
    lua_pushcfunction(m_lua.state(), func_name);	\
	lua_setglobal(m_lua.state(), #func_name);


//------------------------------------------------------------------------------------------
class zlua
{
	lua_State *m_lua_state;

public:
	zlua();
	virtual ~zlua();

	lua_State* state() {return m_lua_state; };

	void new_state();
	void open_libs();
	void close();

	bool load_script(const char *file_name);

	bool call_function(const char *func_name);

private:
	void _log_error();

};
