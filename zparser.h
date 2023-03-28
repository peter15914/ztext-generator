#pragma once

#include "zGenProcs.h"
#include <zstlib/zlua.h>


//------------------------------------------------------------------------------------------
class zParserApp : public boost::noncopyable
{
	zlua m_lua;
	zGenProcs *m_gen_procs;

public:

	zParserApp();
	virtual ~zParserApp();

	//singleton
	static zParserApp &inst();

	//
	zlua &lua() { return m_lua; }
	zGenProcs &gen_procs()
	{
		if(!m_gen_procs)
			m_gen_procs = new zGenProcs();
		return *m_gen_procs;
	}

	void bind_scripts();

	void main_proc();

private:
};

