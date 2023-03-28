#pragma once

/*/----------------------------------------------------------------------------
	@namespace zCfg
	@desc
		чтение файла происходит при первом запросе какого-нить значения

	2008May20 created. zm
-----------------------------------------------------------------------------*/

#include <string>
#include <fstream>
#include <vector>
#include <map>

//------------------------------------------------------------------------------
//для хранения одной строки
struct zCfgStr
{
	enum Z_CNFG_STR_TYPE
	{
		Z_CNFG_INT,
		Z_CNFG_DOUBLE,
		Z_CNFG_STR,
		_Z_CNFG_VARS_,	//для проверки <_Z_CNFG_VARS_ - значит Z_CNFG_INT, Z_CNFG_DOUBLE или Z_CNFG_STR
		//
		//
		Z_CNFG_COMMENT,	//строка без каких-то данных (пустая либо коммент)
		Z_CNFG_PREFIX
	};
	//
	Z_CNFG_STR_TYPE m_Type;
	std::string m_sFullStr;
	std::string m_sParamName;
	//
	int m_iParamVal;
	std::string m_sParamVal;
	double m_dParamVal;
	//
	//для отладки (из-за проблем с совпадающим хешем добавил)
	std::string m_sPrefix;
	//
	zCfgStr() : m_Type(Z_CNFG_COMMENT), m_iParamVal(0), m_dParamVal(0.f) {};
	//
	void ParseString(std::string line);
	std::string GetStrForOutput();
	std::string zCfgStr::GetParamNameEx();
};

enum
{
	ZCFG_NO_LOAD	= 1
};

//------------------------------------------------------------------------------
class zCfg
{
	std::wstring m_wsFileName;
	bool m_bLoaded;
	bool m_bChanged;
	int m_iFlags;

	//необходимо при чтении и сохранении в файл
	std::string m_sPrefix;

	//
	int m_iBackupLev;
	int m_iDeepNotFold;
	std::wstring m_wsBackupFold;
	//
	std::vector<zCfgStr> m_aText;
	//
	std::map<int, int> m_NamesMap;
	//
	zCfg *m_DefaultCfg;
	//
public:
	zCfg(std::wstring _wsFileName = L"", int iFlags = 0);
	virtual ~zCfg();
	//
	void SetFileName(std::wstring _wsFileName);
	std::wstring GetFileName() { return m_wsFileName; }
	void releaseAll();
	//
	void save();
	void save(std::wstring wsFileName);
	void load();
	//
	//новая фича - если есть m_DefaultCfg, то при ненахождении значения в основном cfg, оно ищет в m_DefaultCfg
	void setDefaultCfg(std::wstring wsFileName);
	zCfg *GetDefaultCfg(){ return m_DefaultCfg; }
	//
	int GetValI(std::string sPrefix, std::string sName, int iDfltVal);
	double GetValD(std::string sPrefix, std::string sName, double dDfltVal);
	std::string GetValS(std::string sPrefix, std::string sName, std::string sDfltVal);
	std::wstring GetValSWide(std::string sPrefix, std::string sName, std::string sDfltVal);
	std::string GetValAsS(std::string sPrefix, std::string sName, std::string sDfltVal);
	//
	void SetValI(std::string sPrefix, std::string sName, int iVal);
	void SetValD(std::string sPrefix, std::string sName, double dVal);
	void SetValS(std::string sPrefix, std::string sName, std::string sVal);
	//
	void AddString(zCfgStr &cfgStr);
	void AddPrefix(std::string sPrefix);
	//
	inline int GetStrCnt() { return (int)m_aText.size(); };
	zCfgStr::Z_CNFG_STR_TYPE GetParamType(int iNum);
	std::string GetParamName(int iNum);
	//
	void SetBackupLev(int iBackupLev) { m_iBackupLev = iBackupLev; }
	void SetDeepNotFold(int iDeepNotFold) { m_iDeepNotFold = iDeepNotFold; }
	void SetBackupFold(std::wstring _wsBackupFold) { m_wsBackupFold = _wsBackupFold; }

private:
	//
	friend class zCfgSet;
	zCfgStr *findArg(const std::string &sPrefix, const std::string &sName);
	void AddParam(zCfgStr::Z_CNFG_STR_TYPE _Type, std::string sName);
	inline std::string codeParam(std::string sPrefix, std::string sName) { return sPrefix.empty() ? sName : sPrefix+"#_#"+sName; }
};

