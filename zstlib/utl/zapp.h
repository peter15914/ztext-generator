#pragma once

/*/----------------------------------------------------------------------------
	zApp - приложение с некоторым стандартным функционалом: лог, lua и т.д.

	2012Aug16 created. zm
-----------------------------------------------------------------------------*/

#include <boost/noncopyable.hpp>

#ifndef ZZZ_NO_LUA
	#include <zstlib/zlua.h>
#endif

#include <zstlib/utl/zLog.h>


//------------------------------------------------------------------------------
class zApp : public boost::noncopyable
{
protected:

#ifndef ZZZ_NO_LUA
	zlua m_lua;
#endif

	int m_argc;
	_TCHAR** m_argv;

	bool m_console_app;	//если true, то консольное приложение

	zLog m_log;

	bool m_exit_instance_called;

public:
	//если true, то запускает lua-скрипт, иначе some_func()
	bool m_call_lua;

	std::wstring m_error_log_name;
	bool m_use_debug_break;

protected:
	zApp();
public:
	virtual ~zApp();

	void set_console_app(bool console_app) { m_console_app = console_app; }

	//пользуемся тем, что у нас может быть один app за раз
	static zApp* m_inst;
	static zApp& inst_ref() { return *m_inst; };

	//
#ifndef ZZZ_NO_LUA
	zlua &lua() { return m_lua; }
#endif

	/// для консольных приложений
	//функция вызывается ровно один раз из _tmain()
	virtual int main(int argc, _TCHAR* argv[]);

	/// для неконсольных приложений
	virtual int main_start();
	virtual int main_process();
	virtual int main_finish();

	//для удобства что-то иногда в ней реализовываем
	virtual void some_func();

#ifndef ZZZ_NO_LUA
	//биндаем функции для lua
	virtual void bind_scripts();
#endif

protected:
	//должна вызываться в конце работы программы (чтобы ничего не делать в деструкторах статических объектов)
	virtual void exit_instance();
};
