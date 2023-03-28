#include "stdafx.h"

#include <zstlib/utl/zCfg.h>
#include <zstlib/zhash.h>
#include <zstlib/zstr.h>
#include <zstlib/zutl.h>

#include <zstlib/utl/zLog.h>

#include <zstlib/zDebug.h>


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//------------------------------------------------------------------------------
zCfg::zCfg(std::wstring _swFileName, int iFlags) :
	m_wsFileName(_swFileName),
	m_bLoaded(false),
	m_iFlags(iFlags),
	m_bChanged(false),
	m_DefaultCfg(0)
{
	m_iBackupLev = 2;
	m_iDeepNotFold = 0;
}


//------------------------------------------------------------------------------
zCfg::~zCfg()
{
	if(m_DefaultCfg)
		delete m_DefaultCfg;
	m_DefaultCfg = 0;
}


//------------------------------------------------------------------------------
void zCfg::SetFileName(std::wstring _swFileName)
{
	rel_assert(!m_bLoaded);
	m_wsFileName = _swFileName;
}


//------------------------------------------------------------------------------
void zCfg::releaseAll()
{
	m_wsFileName = L"";
	m_bLoaded = false;
	m_bChanged = false;
	m_iFlags = 0;
	m_aText.clear();
	m_NamesMap.clear();
	if(m_DefaultCfg)
		delete m_DefaultCfg;
	m_DefaultCfg = 0;
}


//------------------------------------------------------------------------------
void zCfg::load()
{
	if(m_bLoaded)
		return;
	m_bLoaded = true;
	//
	if(m_wsFileName.empty() || m_iFlags&ZCFG_NO_LOAD)
		return;
	//
	std::ifstream file(m_wsFileName.c_str());
	if(!file)
		return;
	//
	m_sPrefix = "";
	std::string line;
	while(1)
	{
		std::getline(file, line);
		if(file.eof())
			break;
		//
		zCfgStr cfgStr;
		cfgStr.ParseString(line);
		//
		AddString(cfgStr);
		//
		if(line == "<end_of_file>" || line == "<eof>")
			break;
	}
	//
	m_sPrefix = "";
	//
	file.close();
	//
	m_bChanged = false;
}


//------------------------------------------------------------------------------
void zCfg::save()
{
	save(m_wsFileName);
}


//------------------------------------------------------------------------------
void zCfg::save(std::wstring wsFileName)
{
	if(m_bChanged)
		;//всё ОК, сохраняем
	else
		return;
	//
	if(!m_wsBackupFold.empty())
		zutl::MakeBackupsEx(wsFileName.c_str(), m_iBackupLev, m_wsBackupFold, m_iDeepNotFold);
	else
	if(m_iDeepNotFold == 0)
		zutl::MakeBackups(wsFileName.c_str(), m_iBackupLev);
	else
		zutl::MakeBackupsEx(wsFileName.c_str(), m_iBackupLev, L"_bak", m_iDeepNotFold);
	//
	std::ofstream file(wsFileName.c_str());
	if(!file)
		return;
	//
	m_sPrefix = "";
	for(int i = 0; i < (int)m_aText.size(); i++)
	{
		std::string str = m_aText[i].GetStrForOutput();
		file << str << "\n";
	}
	m_sPrefix = "";
	//
	file.close();
	//
	m_bChanged = false;
}


//------------------------------------------------------------------------------
void zCfgStr::ParseString(std::string line)
{
	m_sFullStr = line;
	m_Type = Z_CNFG_COMMENT;
	//обрубаем комменты
	size_t j = line.find(';');
	if(j != std::string::npos)
	{
		//если не в кавычках, то обрубаем
		int n = 0;
		for(size_t i = 0; i < j; i++)
		{
			if(line[i] == '\"')
				n++;
		}
		if(n%2 == 0)
			line = line.substr(0, j);
	}
	//
	//проверяем, не содержится ли в строке префикс
	size_t j1 = line.find('[');
	size_t j2 = line.find(']');
	//*zm09Aug19 - закомментил (j1 < j2-1), чтоб пустые префиксы работали тоже
	if(j1 != std::string::npos && j2 != std::string::npos && /*j1 < j2-1 && */line.find('=') == std::string::npos)
	{
		m_sParamName = line.substr(j1+1, j2-j1-1);
		m_Type = Z_CNFG_PREFIX;
		return;
	}
	//
	j = line.find('=');
	if(j == std::string::npos)
		return;
	//
	m_sParamName = line.substr(1, j-1);
	if(m_sParamName.length() < 1)
		return;
	//
	if(line[0] == 's') m_Type = Z_CNFG_STR;
	else if(line[0] == 'i') m_Type = Z_CNFG_INT;
	else if(line[0] == 'd') m_Type = Z_CNFG_DOUBLE;
	else return;
	//
	line = line.substr(j+1, line.length());
	//
	if(m_Type == Z_CNFG_STR)
	{
		if(line[0]=='\"')
		{
			size_t pp = line.find_last_of('\"');
			if(pp > 0)
				line = line.substr(1, pp-1);
		}
		m_sParamVal = line;
	}
	else if(m_Type == Z_CNFG_INT)
		m_iParamVal = atoi(line.c_str());
	else if(m_Type == Z_CNFG_DOUBLE)
		m_dParamVal = atof(line.c_str());
}


//------------------------------------------------------------------------------
std::string zCfgStr::GetParamNameEx()
{
	size_t j = m_sParamName.find(m_sPrefix);
	if(j != std::string::npos)
		return m_sParamName.substr(j+m_sPrefix.length(), m_sParamName.length());

	return m_sParamName;
}


//------------------------------------------------------------------------------
std::string zCfgStr::GetStrForOutput()
{
	if(m_Type == Z_CNFG_COMMENT)
		return m_sFullStr;
	if(m_Type == Z_CNFG_INT)
		return zstr::fmt("i%s=%d", GetParamNameEx().c_str(), m_iParamVal);
	if(m_Type == Z_CNFG_DOUBLE)
		return zstr::fmt("d%s=%.3f", GetParamNameEx().c_str(), m_dParamVal);
	if(m_Type == Z_CNFG_STR)
		return zstr::fmt("s%s=\"%s\"", GetParamNameEx().c_str(), m_sParamVal.c_str());
	if(m_Type == Z_CNFG_PREFIX)
	{
		m_sPrefix = m_sParamName;
		return zstr::fmt("[%s]", m_sParamName.c_str());
	}
	//
	rel_assert(0);
	return "";
}


//------------------------------------------------------------------------------
zCfgStr *zCfg::findArg(const std::string &sPrefix, const std::string &sName)
{
	std::string str;
	str = codeParam(sPrefix, sName);
	load();
	//
	zCfgStr *ret = 0;
	std::map<int, int>::iterator buf;
	if((buf = m_NamesMap.find(zhash::GetHash(str.c_str()))) != m_NamesMap.end())
	{
		int pos = buf->second;
		ret = &m_aText[pos];
		dbg_assert(codeParam(ret->m_sPrefix, ret->m_sParamName) == str);	//если тут ассерт, значит хеш-функция фиговая
	}
	//
	if(!ret && m_DefaultCfg)
		return m_DefaultCfg->findArg(sPrefix, sName);
	//
	return ret;
}


//------------------------------------------------------------------------------
void zCfg::AddString(zCfgStr &cfgStr)
{
	if(cfgStr.m_Type == zCfgStr::Z_CNFG_PREFIX)
		m_sPrefix = cfgStr.m_sParamName;
	//
	std::string strForHash = codeParam("", cfgStr.m_sParamName);
	if(cfgStr.m_Type < zCfgStr::_Z_CNFG_VARS_ && !m_sPrefix.empty())
	{
		strForHash = codeParam(m_sPrefix, cfgStr.m_sParamName);
		cfgStr.m_sPrefix = m_sPrefix;
	}
	//
	m_aText.push_back(cfgStr);
	//
	if(cfgStr.m_Type < zCfgStr::_Z_CNFG_VARS_)
	{
		std::pair<int, int> p;
		std::map<int, int>::iterator buf;
		//
		p.first = zhash::GetHash(strForHash.c_str());
		p.second = (int)m_aText.size()-1;
		//
		if((buf = m_NamesMap.find(p.first)) != m_NamesMap.end())
		{
			if(zdebug::log())
			{
				std::string ss;
				if(buf->second >= 0 && buf->second < (int)m_aText.size())
					ss = m_aText[buf->second].m_sPrefix + m_aText[buf->second].m_sParamName;
				zdebug::log()->Error(zstr::fmt("duplicate hashes %s, %s", strForHash.c_str(), ss.c_str()));
			}
			rel_assert(0);
		}
		else
			m_NamesMap.insert(p);
	}
	m_bChanged = true;
}


//------------------------------------------------------------------------------
int zCfg::GetValI(std::string sPrefix, std::string sName, int iDfltVal)
{
	zCfgStr *p = findArg(sPrefix, sName);
	if(!p) return iDfltVal;
	rel_assert(p->m_Type == zCfgStr::Z_CNFG_INT);
	return p->m_iParamVal;
}


//------------------------------------------------------------------------------
double zCfg::GetValD(std::string sPrefix, std::string sName, double dDfltVal)
{
	zCfgStr *p = findArg(sPrefix, sName);
	if(!p) return dDfltVal;
	rel_assert(p->m_Type == zCfgStr::Z_CNFG_DOUBLE);
	return p->m_dParamVal;
}


//------------------------------------------------------------------------------
std::string zCfg::GetValS(std::string sPrefix, std::string sName, std::string sDfltVal)
{
	zCfgStr *p = findArg(sPrefix, sName);
	if(!p) return sDfltVal;
	rel_assert(p->m_Type == zCfgStr::Z_CNFG_STR);
	return p->m_sParamVal;
}


//------------------------------------------------------------------------------
std::wstring zCfg::GetValSWide(std::string sPrefix, std::string sName, std::string sDfltVal)
{
	std::string s = this->GetValS(sPrefix, sName, sDfltVal);
	return zstr::s_to_w(s);
}


//------------------------------------------------------------------------------
std::string zCfg::GetValAsS(std::string sPrefix, std::string sName, std::string sDfltVal)
{
	zCfgStr *p = findArg(sPrefix, sName);
	if(!p) return sDfltVal;
	//
	if(p->m_Type == zCfgStr::Z_CNFG_STR)
		return p->m_sParamVal;
	else
	if(p->m_Type == zCfgStr::Z_CNFG_INT)
		return zstr::fmt("%d", p->m_iParamVal);
	else
	if(p->m_Type == zCfgStr::Z_CNFG_DOUBLE)
		return zstr::fmt("%f", p->m_dParamVal);
	//
	rel_assert(0);
	return sDfltVal;
}


//------------------------------------------------------------------------------
void zCfg::SetValI(std::string sPrefix, std::string sName, int iVal)
{
	zCfgStr *p = findArg(sPrefix, sName);
	if(!p)
	{
		AddPrefix(sPrefix);
		AddParam(zCfgStr::Z_CNFG_INT, sName);
		p = findArg(sPrefix, sName);
	}
	rel_assert(p->m_Type == zCfgStr::Z_CNFG_INT);
	if(p->m_iParamVal != iVal)
	{
		p->m_iParamVal = iVal;
		m_bChanged = true;
	}
}


//------------------------------------------------------------------------------
void zCfg::SetValD(std::string sPrefix, std::string sName, double dVal)
{
	zCfgStr *p = findArg(sPrefix, sName);
	if(!p)
	{
		AddPrefix(sPrefix);
		AddParam(zCfgStr::Z_CNFG_DOUBLE, sName);
		p = findArg(sPrefix, sName);
	}
	rel_assert(p->m_Type == zCfgStr::Z_CNFG_DOUBLE);
	if(p->m_dParamVal != dVal)
	{
		p->m_dParamVal = dVal;
		m_bChanged = true;
	}
}


//------------------------------------------------------------------------------
void zCfg::SetValS(std::string sPrefix, std::string sName, std::string sVal)
{
	zCfgStr *p = findArg(sPrefix, sName);
	if(!p)
	{
		AddPrefix(sPrefix);
		AddParam(zCfgStr::Z_CNFG_STR, sName);
		p = findArg(sPrefix, sName);
	}
	rel_assert(p->m_Type == zCfgStr::Z_CNFG_STR);
	if(p->m_sParamVal != sVal)
	{
		p->m_sParamVal = sVal;
		m_bChanged = true;
	}
}


//------------------------------------------------------------------------------
void zCfg::AddParam(zCfgStr::Z_CNFG_STR_TYPE _Type, std::string sName)
{
	zCfgStr cfgStr;
	cfgStr.m_Type = _Type;
	cfgStr.m_sParamName = sName;
	AddString(cfgStr);
}

//------------------------------------------------------------------------------
void zCfg::AddPrefix(std::string sPrefix)
{
	if(sPrefix == m_sPrefix)
		return;
	//
	zCfgStr cfgStr;
	cfgStr.m_Type = zCfgStr::Z_CNFG_PREFIX;
	cfgStr.m_sParamName = sPrefix;
	AddString(cfgStr);
}


//------------------------------------------------------------------------------
zCfgStr::Z_CNFG_STR_TYPE zCfg::GetParamType(int iNum)
{
	dbg_assert(iNum >= 0 && iNum < GetStrCnt());
	return m_aText[iNum].m_Type;
}


//------------------------------------------------------------------------------
std::string zCfg::GetParamName(int iNum)
{
	dbg_assert(iNum >= 0 && iNum < GetStrCnt());
	return m_aText[iNum].m_sParamName;
}


//------------------------------------------------------------------------------
void zCfg::setDefaultCfg(std::wstring wsFileName)
{
	if(m_DefaultCfg)
		delete m_DefaultCfg;
	m_DefaultCfg = 0;
	//
	m_DefaultCfg = new zCfg(wsFileName);
}

