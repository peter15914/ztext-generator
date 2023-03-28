#include "stdafx.h"

#include "zparser.h"



//------------------------------------------------------------------------------------------
static int lua_Log(lua_State *L)
{
	const char *s = lua_tostring(L, 1);	//get argument
	zdebug::log()->Log(s);
	return 0;		//number of results
}


//------------------------------------------------------------------------------------------
static int lua_LogImp(lua_State *L)
{
	const char *s = lua_tostring(L, 1);	//get argument
	zdebug::log()->LogImp(s);
	return 0;		//number of results
}


//------------------------------------------------------------------------------------------
static int lua_gen_procs__test_gen_sentences(lua_State *L)
{
	zParserApp::inst().gen_procs().test_gen_sentences();
	return 0;		//number of results
}


//------------------------------------------------------------------------------------------
static int lua_gen_procs__gen_sentences(lua_State *L)
{
	const char *s = lua_tostring(L, 1);			//get argument
	int sent_cnt = lua_tointeger(L, 2);			//get argument
	int sent_len_min = lua_tointeger(L, 3);		//get argument
	int sent_len_max = lua_tointeger(L, 4);		//get argument

	zParserApp::inst().gen_procs().gen_sentences(zstr::s_to_w(s).c_str(), sent_cnt, sent_len_min, sent_len_max);
	return 0;		//number of results
}


//------------------------------------------------------------------------------------------
static int lua_gen_procs__show_w3db_usage(lua_State *L)
{
	zParserApp::inst().gen_procs().show_w3db_usage();
	return 0;		//number of results
}


//------------------------------------------------------------------------------------------
static int lua_gen_procs__gen_with_first_pair(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);			//get argument
	const char *s2 = lua_tostring(L, 2);			//get argument
	const char *s3 = lua_tostring(L, 3);			//get argument
	const char *s4 = lua_tostring(L, 4);			//get argument

	bool ok = zParserApp::inst().gen_procs().gen_with_first_pair(s1, s2, s3, s4);
	rel_assert(ok);

	return 0;		//number of results
}


//------------------------------------------------------------------------------------------
void zParserApp::bind_scripts()
{
	lua_pushcfunction(m_lua.state(), lua_Log);
    lua_setglobal(m_lua.state(), "Log");

    lua_pushcfunction(m_lua.state(), lua_LogImp);
    lua_setglobal(m_lua.state(), "LogImp");

    lua_pushcfunction(m_lua.state(), lua_gen_procs__test_gen_sentences);
    lua_setglobal(m_lua.state(), "gen_procs__test_gen_sentences");

    lua_pushcfunction(m_lua.state(), lua_gen_procs__gen_sentences);
    lua_setglobal(m_lua.state(), "gen_procs__gen_sentences");

	lua_pushcfunction(m_lua.state(), lua_gen_procs__show_w3db_usage);
    lua_setglobal(m_lua.state(), "gen_procs__show_w3db_usage");

	lua_pushcfunction(m_lua.state(), lua_gen_procs__gen_with_first_pair);
    lua_setglobal(m_lua.state(), "gen_procs__gen_with_first_pair");
}
