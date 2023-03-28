#include "stdafx.h"

#include "zhtmlapp.h"

#include <zstlib/utl/zLog.h>
#include <zstlib/zutl.h>


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;


zHtmlApp* zHtmlApp::m_hmtlinst = 0;

//------------------------------------------------------------------------------------------
zHtmlApp::zHtmlApp()
{
	zHtmlApp::m_hmtlinst = this;
}


//------------------------------------------------------------------------------------------
zHtmlApp::~zHtmlApp()
{
}


//------------------------------------------------------------------------------------------
//функция вызывается ровно один раз из _tmain()
int zHtmlApp::main(int argc, _TCHAR* argv[])
{
	int ret = super::main(argc, argv);

	return ret;
}


//------------------------------------------------------------------------------------------
//для удобства что-то иногда в ней реализовываем
void zHtmlApp::some_func()
{
}


//------------------------------------------------------------------------------------------
static int lua_zclean__parse_html_file(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);
	zHtmlApp::inst_ref().m_cleaner.parse_html_file(s1);
	return 0;
}


//------------------------------------------------------------------------------------------
static int lua_zclean__insert_rand_tags(lua_State *L)
{
	zHtmlApp::inst_ref().m_cleaner.insert_rand_tags();
	return 0;
}


//------------------------------------------------------------------------------------------
static int lua_zclean__replace_ahref_titles(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);
	_ass(0);	//todo
	//zHtmlApp::inst_ref().m_cleaner.replace_ahref_titles(zstr::s_to_w(s1).c_str());
	return 0;
}


//------------------------------------------------------------------------------------------
static int lua_zclean__save_to_file(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);
	const char *s2 = lua_tostring(L, 2);
	zHtmlApp::inst_ref().m_cleaner.save_to_file(s1, s2);
	return 0;
}


//------------------------------------------------------------------------------------------
static int lua_zclean__clean_css_dir(lua_State *L)
{
	const char *s1 = lua_tostring(L, 1);
	const int i2 = lua_tointeger(L, 2);
	const int i3 = lua_tointeger(L, 3);
	zHtmlApp::inst_ref().m_cleaner.clean_css_dir(zstr::s_to_w(s1).c_str(), i2, i3);
	return 0;
}


//------------------------------------------------------------------------------------------
static int lua_zclean__remove_blank_lines_and_trim(lua_State *L)
{
	_ass(0);	//todo
	//zHtmlApp::inst_ref().m_cleaner.remove_blank_lines_and_trim();
	return 0;
}


//------------------------------------------------------------------------------------------
static int lua_zclean__set_backups_depth(lua_State *L)
{
	const int i1 = lua_tointeger(L, 1);
	zHtmlApp::inst_ref().m_cleaner.set_backups_depth(i1);
	return 0;
}


//------------------------------------------------------------------------------------------
//биндаем функции для lua
void zHtmlApp::bind_scripts()
{
	super::bind_scripts();

	ZLUA_BIND_FUNC(lua_zclean__parse_html_file);
	ZLUA_BIND_FUNC(lua_zclean__insert_rand_tags);
	ZLUA_BIND_FUNC(lua_zclean__replace_ahref_titles);
	ZLUA_BIND_FUNC(lua_zclean__save_to_file);
	ZLUA_BIND_FUNC(lua_zclean__clean_css_dir);
	ZLUA_BIND_FUNC(lua_zclean__remove_blank_lines_and_trim);
	ZLUA_BIND_FUNC(lua_zclean__set_backups_depth);
}

