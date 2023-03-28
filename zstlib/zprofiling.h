#pragma once

/*/----------------------------------------------------------------------------
	@desc
		макросы для удобного профайлинга

	2007Jan03 created. zm
-----------------------------------------------------------------------------*/

#ifdef NO_PROFILING
#define PROF_DECLARE_EXTERN_VARS()              ;
#define PROFILING_DECL(mode, enable)			((void)0)
#define TURN_SILENCE_AFTER(time, mode)			((void)0)
#define BEGIN_PROFILING(desc)					((void)0)
#define SET_LOG_OUTPUT_IF(exp,newmode)			((void)0)
#define SET_LOG_OUTPUT(newmode)					((void)0)
#define STEP_PROFILING(strinfo, varmax)			((void)0)
#define PROFILING_LOG(str, intvar)				((void)0)
#define END_PROFILING()							((void)0)
#define CLEAR_PROF_STAT(varmax)					((void)0)
#define CLEAR_PROF_STAT_ONCE(varmax)			((void)0)
#define CLEAR_CUR_STEP()						((void)0)
#define TOGGLE_PROFILE_MODE()					((void)0)

#else
//------------------------------------------------------------------------------
//g_iSilence - отвечает за вывод в лог
//	mode==0 - не выводить в лог.
//	mode==1 - выводить в лог постоянно.
//	mode==2 - вывести один раз и сбросить mode в ноль (в END_PROFILING)
//g_bEnabled - ведется ли профайлинг вообще

//g_iSilenceTurn и g_bEnabledTurn нужны чтобы врубать на след. такте
extern int g_iSilenceTurn;
extern bool g_bEnabledTurn;

//------------------------------------------------------------------------------
#define PROF_DECLARE_EXTERN_VARS()											\
	int g_iSilenceTurn = -1;												\
	bool g_bEnabledTurn = false;

//------------------------------------------------------------------------------
#define PROFILING_DECL(mode, enable)										\
		static int g_iSilence = mode;										\
		static bool g_bEnabled = enable;									\
		g_iSilenceTurn = -1;												\
		g_bEnabledTurn = false;

//------------------------------------------------------------------------------
/*#define TURN_SILENCE_AFTER(time, mode)										\
{																			\
	if(g_bEnabled)															\
	{																		\
		static int s_iBeginTime = zutl::GetCurTimeU();						\
		if(g_iSilence == 2) g_iSilence = 0;									\
		{if( s_iBeginTime && zutl::GetCurTimeU() - s_iBeginTime > time )	\
		{																	\
			g_iSilenceTurn = mode;											\
			s_iBeginTime = 0;												\
		};																	\
	}																		\
}*/

//------------------------------------------------------------------------------
#define BEGIN_PROFILING(desc)												\
	int PF_t0, PF_t2, PF_tb, PF_dt;											\
{																			\
	if(g_bEnabledTurn)														\
	{																		\
		g_bEnabled = !g_bEnabled;											\
		g_bEnabledTurn = false;												\
	}																		\
	if(g_iSilenceTurn != -1)												\
	{																		\
		g_iSilence = g_iSilenceTurn;										\
		g_iSilenceTurn = -1;												\
	}																		\
	if(g_bEnabled)															\
	{																		\
		PF_t0 = zutl::GetSuperTimer();										\
		PF_tb = zutl::GetCurTimeU();										\
		if(g_iSilence)														\
			::Error->Log(desc);												\
	}																		\
}

//------------------------------------------------------------------------------
#define SET_LOG_OUTPUT(newmode)												\
		g_iSilenceTurn = newmode;

#define TOGGLE_PROFILE_MODE()												\
		g_bEnabledTurn = true;

//------------------------------------------------------------------------------
//PROFILING_LEN_BY_TEST - число, полученное опытным путем функцией GetProfTime_Test()
#define PROFILING_LEN_BY_TEST	5
#define STEP_PROFILING(strinfo, varmax)										\
{																			\
	if(g_bEnabled)															\
	{																		\
		PF_t2 = zutl::GetSuperTimer();										\
		static int varmax = 0;												\
		static int avgN##varmax = 0, avgSum##varmax = 0;					\
		PF_dt = PF_t2-PF_t0;												\
		if(PF_dt > varmax) varmax = PF_dt;									\
		avgSum##varmax += PF_dt; avgN##varmax ++;							\
		if(g_iSilence) {													\
			::Error->Log(strinfo": cur: %d. max: %d. avg: %d",				\
			PF_dt-PROFILING_LEN_BY_TEST, varmax-PROFILING_LEN_BY_TEST,		\
			avgSum##varmax/avgN##varmax - PROFILING_LEN_BY_TEST); }			\
		PF_t0 = PF_t2;														\
	}																		\
}

//------------------------------------------------------------------------------
#define CLEAR_CUR_STEP()													\
{																			\
	if(g_bEnabled)															\
	{																		\
		PF_t2 = zutl::GetSuperTimer();										\
		PF_t0 = PF_t2;														\
	}																		\
}

//------------------------------------------------------------------------------
//#define PROFILING_LOG(str, intvar) {if(g_iSilence) ::Error->Log(str, intvar);}

//------------------------------------------------------------------------------
#define END_PROFILING()																		\
{																							\
	if(g_bEnabled)																			\
	{																						\
		int buf = zutl::GetCurTimeU()-PF_tb; if(buf <= 0 ) buf = 0;							\
		if(g_iSilence)																		\
		{																					\
			::Error->Log("----total time: %d. fps: %d", buf, buf ? 1000/ buf : 0 );		\
			if(g_iSilence == 2) g_iSilence = 0;												\
		}																					\
	}																						\
}

//------------------------------------------------------------------------------
#define CLEAR_PROF_STAT(varmax)												\
{																			\
	if(g_bEnabled)															\
	{																		\
		varmax = 0; avgN##varmax = 0; avgSum##varmax = 0;					\
	}																		\
}

//------------------------------------------------------------------------------
#define CLEAR_PROF_STAT_ONCE(varmax)										\
{																			\
	if(g_bEnabled)															\
	{																		\
		static bool bbb##varmax = false;									\
		if(!bbb##varmax)													\
			{varmax = 0; avgN##varmax = 0; avgSum##varmax = 0;}				\
		bbb##varmax = true;													\
	}																		\
}

//------------------------------------------------------------------------------

#endif

