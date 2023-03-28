#pragma once

/*/----------------------------------------------------------------------------
	@namespace zutl
	@desc
		zutl  - некоторые вспомогательные функции

	2007Jan03 created. zm
-----------------------------------------------------------------------------*/

#include <set>

//------------------------------------------------------------------------------
namespace zutl
{
	bool FileExists(const std::wstring &wsFileName);
	//strict - дополнительная проверка, т.к. по маске лишние находит
	void FindFilesInFolder(const wchar_t *_work_dir, const wchar_t *mask, std::vector<std::wstring> &files, bool strict = true);

	void MakeBackups(const std::wstring &wsFileName, int iNum);
	void MakeBackupsEx(const std::wstring &wsFilePath, int iDeep, const std::wstring &wsBakFoldName, int iDeepNotFold);

	//возвращает размер файла в байтах
	//__int64 get_file_size(const std::wstring &file_name);
	__int64 get_file_size(const wchar_t *file_name);

	//заменяет в файле все строки s_old на s_new
	//возвращает количество замен
	//если backup_deep == -1, то не делаем бэкап
	int replace_substr_in_file(const wchar_t* file_name, const char* s_old, const char* s_new, int backup_deep = -1);

	//аналогично replace_substr_in_file, но вместо s_new берет полностью содержимое файла src_file_name
	int replace_substr_in_file_from_file(const wchar_t* file_name, const char* s_old, const wchar_t* src_file_name,
																								int backup_deep = -1);

	//помещает содержимое файла в set<string>
	void text_file_to_set(const wchar_t* file_name, std::set<std::string> &strings);

	//
	//std::string GenUniqFName(const char *new_name);
	//
	std::string GetTimeStr();
	std::string GetDateTimeStrFull();//миллисекунды - ненастоящие
	std::string GetDateStr();

	unsigned long GetCurTimeU();
	int QueryPerfFreq();
	int GetSuperTimer();	//супер точный таймер. неопределенные единицы измерения (НЕИ)

	//возвращает примерное кол-во НЕИ, кот. тратятся на один STEP_PROFILING
	int GetProfTime_Test();
	//
	bool LaunchApp(const std::wstring &wsExeName, const std::wstring &wsParam, std::wstring wsAppDir, bool bAutoAppDir, bool bWaitProcess);

	//проверяет, что _NO_DEBUG_HEAP=1
	void check_debug_heap();
};
