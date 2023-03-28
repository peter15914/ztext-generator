#pragma once

/*/----------------------------------------------------------------------------
	zHtmlApp - расширенное приложение, с функциями для работы с html

	2012Dec06 created. zm
-----------------------------------------------------------------------------*/

#include "zapp.h"

#include <zstlib/html/zcleaner.h>


//------------------------------------------------------------------------------
class zHtmlApp : public zApp
{
	typedef zApp super;

public:
	zcleaner m_cleaner;

public:
	zHtmlApp();
	virtual ~zHtmlApp();

	//пользуемся тем, что у нас может быть один app за раз
	static zHtmlApp* m_hmtlinst;
	static zHtmlApp& inst_ref() { return *m_hmtlinst; };

	//функция вызывается ровно один раз из _tmain()
	virtual int main(int argc, _TCHAR* argv[]);

	//для удобства что-то иногда в ней реализовываем
	virtual void some_func();

	//биндаем функции для lua
	virtual void bind_scripts();

protected:

};
