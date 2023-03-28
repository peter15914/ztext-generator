#include "stdafx.h"

#include "zSpeedWindowsH.h"

#include <Windows.h>		//для timeGetTime
#include <Mmsystem.h>		//для timeGetTime
#include <time.h>
#include <io.h>
#include <direct.h>			//for _mkdir

#include <vector>

#include "zutl.h"
#include <zstlib/zstr.h>

#include <zstlib/zDebug.h>


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#pragma comment(lib, "winmm.lib")

//------------------------------------------------------------------------------------------
bool zutl::FileExists(const std::wstring &wsFileName)
{
	struct _wfinddata_t n_dir;
	intptr_t hFile = _wfindfirst(wsFileName.c_str(), &n_dir);
	bool ret = ( hFile != -1L );
	_findclose( hFile );
	return ret;
}


//------------------------------------------------------------------------------------------
/*void zutl::MoveToTrash(const char *filename)
{
	if(filename && FileExists(filename) )
	{
		std::string NewFName( std::string(filename) + ".bak" );
		if(FileExists(NewFName.c_str()))
			DeleteFileA(NewFName.c_str());
		MoveFileA(filename, NewFName.c_str());
	}
}*/


//------------------------------------------------------------------------------------------
void zutl::MakeBackups(const std::wstring &wsFileName, int iNum)
{
	dbg_assert(iNum <= 50 && iNum >= 0);

	struct MB
	{
		static std::wstring getBackFileName(const std::wstring &wsFileName, int iNum)
		{
			return zstr::wfmt(L"%s%02d.bak", wsFileName.c_str(), iNum);
		};
	};

	while(iNum > 1)
	{
		std::wstring bakFile1 = MB::getBackFileName(wsFileName, iNum-1);
		std::wstring bakFile2 = MB::getBackFileName(wsFileName, iNum);

		MoveFileW(bakFile1.c_str(), bakFile2.c_str());

		iNum--;
	}

	CopyFileW(wsFileName.c_str(), MB::getBackFileName(wsFileName, 1).c_str(), false);
}


//------------------------------------------------------------------------------------------
/*void zutl::MakeBackups(const char *filename)
{
	MakeBackups(filename, 2);
}*/


//------------------------------------------------------------------------------------------
void zutl::MakeBackupsEx(const std::wstring &wsFilePath, int iDeep, const std::wstring &wsBakFoldName, int iDeepNotFold)
{
	if(!zutl::FileExists(wsFilePath.c_str()))
		return /*(rel_assert(0))*/;		//без ассерта лучше, можно удалять файлы руками
	rel_assert(iDeep > 0);
	//
	std::wstring wsFoldName, wsFileName;
	size_t jj = wsFilePath.find_last_of(L"\\/");
	if(jj == std::string::npos)
		wsFileName = wsFilePath;
	else
	{
		wsFoldName = wsFilePath.substr(0, jj+1);
		wsFileName = wsFilePath.substr(jj+1, wsFilePath.length());
	}
	//
	bool need_folder = false;

	std::vector<std::wstring> aNames;
	for(int i = 0; i < iDeep; i++)
	{
		std::wstring str;
		str = wsFileName + zstr::wfmt(L"%02d.bak", i+1);
		if(!wsBakFoldName.empty() && i >= iDeepNotFold)
		{
			str = wsFoldName + wsBakFoldName + L"/" + str;
			if(!need_folder && (i == 0 || FileExists(aNames[i-1])))
				need_folder = true;
		}
		else
			str = wsFoldName + str;
		aNames.push_back(str);
	}
	//
	if(!wsBakFoldName.empty() && need_folder)
		_wmkdir((wsFoldName + wsBakFoldName).c_str());
	//
	for(int i = iDeep-1; i > 0; i--)
	{
		rel_assert(!aNames[i-1].empty() && !aNames[i].empty());
		MoveFileW(aNames[i-1].c_str(), aNames[i].c_str());
	}
	//
	CopyFileW(wsFilePath.c_str(), aNames[0].c_str(), false);
}


//------------------------------------------------------------------------------------------
//возвращает размер файла в байтах
__int64 zutl::get_file_size(const wchar_t *file_name)
{
	FILE *file1 = _wfopen(file_name, L"rb");
	if(!file1)
		return (rel_assert(0), 0);

	//размер файла
	_fseeki64(file1 , 0 , SEEK_END);
	__int64 fsize1 = _ftelli64(file1);

	fclose(file1);
	return (fsize1 >= 0) ? fsize1 : 0;
}


//------------------------------------------------------------------------------------------
std::string zutl::GetTimeStr()
{
    time_t ltime;
    time( &ltime );
	tm today;
	errno_t err = localtime_s(&today, &ltime);
	if(err)
		return std::string();
	//
    static char buf[256];
    strftime(buf, sizeof(buf), "%H:%M:%S", &today);
	//
	return std::string(buf);
}


//------------------------------------------------------------------------------------------
std::string zutl::GetDateStr()
{
    time_t ltime;
    time( &ltime );
	tm today;
	errno_t err = localtime_s(&today, &ltime);
	if(err)
		return std::string();
	//
    static char buf[256];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &today);
	//
	return std::string(buf);
}


//------------------------------------------------------------------------------------------
std::string zutl::GetDateTimeStrFull()
{
	SYSTEMTIME lt;
	GetLocalTime(&lt);

	return zstr::fmt("%04d-%02d-%02d-%02dh%02dm%02ds%03d",
				lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
}


//------------------------------------------------------------------------------------------
unsigned long zutl::GetCurTimeU()
{
	return timeGetTime();
}


//------------------------------------------------------------------------------------------
int zutl::QueryPerfFreq()
{
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	return (int)Frequency.QuadPart;
}


//------------------------------------------------------------------------------------------
//не использовать в крит. местах - только для отладки, т.к.
//QuadPart в int не помещаться должна порою
int zutl::GetSuperTimer()
{
	LARGE_INTEGER PerformanceCount;
	QueryPerformanceCounter(&PerformanceCount);
	return (int)PerformanceCount.QuadPart/* / 0xff*/;
}


//------------------------------------------------------------------------------------------
int zutl::GetProfTime_Test()
{
	int t0 = zutl::GetSuperTimer();
	int t2;
	//
	static int varmax = 0;
	static int avgNvarmax = 0, avgSumvarmax = 0;
	//
	int n = 1000000;
	for(int i = 0; i < n; i++)
	{
		t2 = zutl::GetSuperTimer();
		if(t2-t0>varmax) varmax = t2-t0;
		avgSumvarmax += (t2-t0); avgNvarmax++;
		t0 = t2;
	}
	//
	return avgSumvarmax / (n/1000);
}


//------------------------------------------------------------------------------
//если понадобится, то возможно стоит генерить addon рэндомом - чтоб тормозов не было
/*std::string zutl::GenUniqFName(const char *new_name)
{
	std::string buf;
	//
	int addon = 0;
	while(true)
	{
		buf = new_name;
		buf += zstr::itoa( addon++, 10 );
		if( !FileExists(buf.c_str()) )
			break;
	}
	return buf;
}
*/


//------------------------------------------------------------------------------------------
bool zutl::LaunchApp(const std::wstring &wsExeName, const std::wstring &wsParam, std::wstring wsAppDir, bool bAutoAppDir, bool bWaitProcess)
{
	if(bAutoAppDir)
	{
		_ass(wsAppDir.empty());

		size_t jj = wsExeName.find_last_of(L"\\/");
		if(jj == std::string::npos)
			return (_ass(0), false);

		wsAppDir = wsExeName.substr(0, jj+1);
	}

	//ShellExecute(NULL, NULL, sExeName.c_str(), sParam.c_str(), NULL, SW_SHOW);
	//
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	std::wstring cmdLine = wsExeName + L" " + wsParam;
	static wchar_t _cmdLine[MAX_PATH*2];
	wchar_t *pCmdLine = &_cmdLine[0];
	wcscpy_s(pCmdLine, sizeof(_cmdLine), cmdLine.c_str());

	//g_log->Log(cmdLine);
    if( !CreateProcessW( NULL,   // No module name (use command line)
		pCmdLine,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
		wsAppDir.empty() ? NULL : wsAppDir.c_str(),           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
		//g_log->Error( zstr::fmt("CreateProcess failed (%d)", GetLastError()) );
        return false;
    }
	//
	if(bWaitProcess)
		WaitForSingleObject( pi.hProcess, INFINITE );
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	//
	return true;
}


//------------------------------------------------------------------------------------------
//strict - дополнительная проверка, т.к. по маске лишние находит
void zutl::FindFilesInFolder(const wchar_t *_work_dir, const wchar_t *mask, std::vector<std::wstring> &files, bool strict/* = true*/)
{
	using namespace std;

	wstring work_mask = _work_dir;
	work_mask += L"/";
	work_mask += mask;

	//strict
	wstring must_end;
	if(strict)
	{
		must_end = mask;
		size_t jj = must_end.find(L'.');
		if(jj != string.npos)
		{
			must_end = must_end.substr(jj, INT_MAX);
			zstr::ToUpper(must_end);
		}

		if(must_end.find('*') != string.npos || must_end.find('?') != string.npos)
			strict = false;
	}
	int m_len = (int)must_end.length();

	//поиск
	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind = FindFirstFileW(work_mask.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
		return (_ass(0));
	
	for(;;)	
	{
		wstring fname = FindFileData.cFileName;

		if(! (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if(strict && m_len)
			{
				if(zstr::ToUpper_(fname).substr(fname.length() - m_len, INT_MAX) == must_end)
					files.push_back(fname);
			}
			else
				files.push_back(fname);
		}

		if(!FindNextFileW(hFind, &FindFileData)) 
			break;
	}

	FindClose(hFind);
}


//------------------------------------------------------------------------------------------
//заменяет в файле все строки s_old на s_new
//возвращает количество замен
int zutl::replace_substr_in_file(const wchar_t* file_name, const char* s_old, const char* s_new,
									 int backup_deep/* = -1*/)
{
	using namespace std;

	int cnt = 0;

	FILE *f = _wfopen(file_name, L"rt");
	if(!f)
		return (_ass(0), 0);

	vector<string> data;
	data.reserve(1000);

	static char buf[10000];
	while(fgets(buf, sizeof(buf), f))
	{
		zstr::remove_eoln(buf);

		//проверяем, нет ли там
		string str = buf;
		boost::replace_all(str, s_old, s_new);
		if(str != buf)
			cnt++;

		data.push_back(str);
	}
	fclose(f);

	//если ничего не заменили - уходим
	if(!cnt)
		return 0;

	if(backup_deep >= 0)
		zutl::MakeBackups(file_name, backup_deep);

	FILE *f2 = _wfopen(file_name, L"wt");

	for(int i = 0; i < (int)data.size(); i++)
		fputs((data[i] + "\n").c_str(), f2);

	fclose(f2);

	return cnt;
}


//------------------------------------------------------------------------------------------
//помещает содержимое файла в set<string>
void zutl::text_file_to_set(const wchar_t* file_name, std::set<std::string> &strings)
{
	FILE *f = _wfopen(file_name, L"rt");
	if(!f)
		return (_ass(0));

	static char buf[10000];
	while(fgets(buf, sizeof(buf), f))
	{
		zstr::remove_eoln(buf);
		strings.insert(buf);
	}
	fclose(f);
}


//------------------------------------------------------------------------------------------
//аналогично replace_substr_in_file, но вместо s_new берет полностью содержимое файла src_file_name
int zutl::replace_substr_in_file_from_file(const wchar_t* file_name, const char* s_old, const wchar_t* src_file_name,
																							int backup_deep/* = -1*/)
{
	using namespace std;

	string s_new;
	s_new.reserve(128);

	//читаем содержимое файла src_file_name
	FILE *f = _wfopen(src_file_name, L"rt");
	if(!f)
		return (_ass(0), 0);

	static char buf[10000];
	while(fgets(buf, sizeof(buf), f))
		s_new += buf;
	fclose(f);

	//
	return replace_substr_in_file(file_name, s_old, s_new.c_str(), backup_deep);
}


//------------------------------------------------------------------------------------------
//проверяет, что _NO_DEBUG_HEAP=1
void zutl::check_debug_heap()
{
	if(IsDebuggerPresent())
	{
		const char* no_debug_env = getenv("_NO_DEBUG_HEAP");
		if(!no_debug_env || strcmp(no_debug_env, "1"))
			_ass(0);
	}
}
