#include "stdafx.h"
#include "zTextParser.h"

#include <zstlib/zutl.h>
#include <zstlib/textutl.h>


//------------------------------------------------------------------------------------------
zTextParser::zTextParser() :
	m_iCurInd(std::string::npos),
	m_isFb2(false),
	m_bUTF8(false),
	m_bTagOpened(false),
	m_PrevRet(LT_NONE),
	m_bSectionOpened(false),
	m_bXmlTagOpened(false),
	m_iEncoding(0)
{
}


//------------------------------------------------------------------------------------------
zTextParser::~zTextParser()
{
	if(m_file.is_open())
		m_file.close();
}


//------------------------------------------------------------------------------------------
void zTextParser::_init_after_open()
{
	m_Buf = "";
	m_iCurInd = std::string::npos;
	m_isFb2 = false;
	m_bUTF8 = false;
	m_bTagOpened = false;
	m_PrevRet = LT_NONE;
	m_bSectionOpened = false;
	m_bXmlTagOpened = false;
	m_iEncoding = 0;
}


//------------------------------------------------------------------------------------------
bool zTextParser::open(const std::wstring &file_name, bool is_fb2)
{
	if(m_file.is_open())
	{
		m_file.close();
		m_file.clear();
	}

	m_file.open(file_name.c_str());
	_init_after_open();

	m_isFb2 = is_fb2;


	return m_file.is_open();
}


//------------------------------------------------------------------------------------------
static bool inline _is_space(char c)
{
	return c == ' ' || c == '\t' || c == '"' || c == '(' || c == ')' || c == ':' || c == '+' || c == ',' || c == ';'
					 || c == (char)160	//160 - это тоже какой-то пробел
					 || c == (char)133	//133 - это тоже ':'
					 || c == (char)132	//132 - это тоже '"'
					 || c == (char)171	//171 - это открывающая кавычка, переведенная из utf-8
					 || c == (char)187	//187 - это закрывающая кавычка, переведенная из utf-8
					 || c == '\''
					 || c == '*';
}

static bool inline _is_dot(char c)
{
	return c == '.' || c == '!' || c == '?';
}


//------------------------------------------------------------------------------------------
zTextParser::LEX_TYPE zTextParser::get_next_word(std::string &ret_str)
{
	LEX_TYPE ret = _get_next_word(ret_str);
	while(m_PrevRet == LT_SENTENCE_END && ret == LT_SENTENCE_END)
	{
		LEX_TYPE ret2 = _get_next_word(ret_str);
		ret = ret2;
	}

	m_PrevRet = ret;
	return m_PrevRet;
}


//------------------------------------------------------------------------------------------
static bool is_eq_strs(const std::string &buf, size_t curInd, const char *str2)
{
	size_t len = buf.length();

	size_t k = curInd;
	for(const char *pp = str2; (*pp) && k < len; pp++)
	{
		char c = buf[k];
		if(textutl::get_lower_eng(c) != *pp)
			return false;
		k++;
	}

	return true;
}


//------------------------------------------------------------------------------------------
zTextParser::LEX_TYPE zTextParser::_get_next_word(std::string &ret_str)
{
	ret_str = std::string();

	while(1)
	{
		if(m_iCurInd == std::string::npos || m_iCurInd >= m_Buf.length())
		{
			std::getline(m_file, m_Buf);
			m_iCurInd = 0;

			if(!m_file)
				return LT_FILE_END;

			//в режиме m_bUTF8 преобразовываем строку
			if(m_bUTF8)
				m_Buf = zstr::utf8_to_ansi(m_Buf);

		}

		//ищем закрывающий тэг
		if(m_isFb2 && m_bTagOpened)
		{
			//проверяем, не встречается ли encoding="utf-8"
			if(m_bXmlTagOpened && !m_bUTF8)
			{
				textutl::to_lower_eng(m_Buf, m_iCurInd);
				size_t jj = m_Buf.find("encoding", m_iCurInd);
				if(jj == std::string.npos)
				{
					rel_assert(0);
					return LT_FILE_END;
				}
				else
				{
					size_t jj2 = m_Buf.find("utf-8", jj+9);
					if(jj2 != std::string.npos)
						m_bUTF8 = true;
					else
					{
						jj2 = m_Buf.find("windows-125", jj+9);

						if(jj2 != std::string.npos)
						{
							if(jj2 + 11 < m_Buf.length())
								m_iEncoding = 1250 + (m_Buf[jj2 + 11] - '0');
						}
						else
						{
							//других кодировок мы не знаем
							//rel_assert(0);
							zdebug::log()->LogImp("!!!bad encoding!!!");
							zdebug::log()->LogImp(m_Buf.substr(jj, jj+30));
							return LT_FILE_END;
						}
					}
				}
			}
			
			//
			while(m_iCurInd < m_Buf.length() && m_Buf[m_iCurInd] != '>')
				m_iCurInd++;

			if(m_iCurInd < m_Buf.length() && m_Buf[m_iCurInd] == '>')
			{
				m_bTagOpened = false;
				m_bXmlTagOpened = false;
				m_iCurInd++;
			}
		}

		//пропускаем пробелы и запятые
		while(m_iCurInd < m_Buf.length() && _is_space(m_Buf[m_iCurInd]))
			m_iCurInd++;

		//переходим к след. строчке, если закончилось
		if(m_iCurInd >= m_Buf.length())
			continue;

		//если точка, то это конец предложения
		if(_is_dot(m_Buf[m_iCurInd]))
		{
			m_iCurInd++;
			if(m_bSectionOpened)
				return LT_SENTENCE_END;
			else
				continue;
		}

		//если открывающий тэг
		if(m_isFb2 && m_Buf[m_iCurInd] == '<')
		{
			m_bTagOpened = true;
			m_iCurInd++;

			//тэг <? (для <?xml)
			if(!m_bSectionOpened && m_Buf[m_iCurInd] == '?')
			{
				m_bXmlTagOpened = true;
				continue;
			}

			//тэг </p> считаем точкой
			if(m_bSectionOpened && is_eq_strs(m_Buf, m_iCurInd, "/p>"))
			{
				return LT_SENTENCE_END;
			}
			else
			if(is_eq_strs(m_Buf, m_iCurInd, "section>"))
				m_bSectionOpened = true;
			else
			if(is_eq_strs(m_Buf, m_iCurInd, "/section>"))
				m_bSectionOpened = false;

			continue;
		}

		//ищем конец слова
		for(size_t i = m_iCurInd; i <= m_Buf.length(); i++)
		{
			if(i == m_Buf.length() || _is_space(m_Buf[i]) || _is_dot(m_Buf[i]) || m_isFb2 && m_Buf[i] == '<')
			{
				if(m_bSectionOpened || !m_isFb2)
				{
					//не учитываем слова, состоящие из одного "-"
					if(i - m_iCurInd == 1)
					{
						char cc = m_Buf[m_iCurInd];
						if(cc == '-' || cc == (char)151 || cc == (char)150)	//151 -это тоже какой-то '-'
						{
							m_iCurInd = i;
							break;
						}
					}

					ret_str = m_Buf.substr(m_iCurInd, i-m_iCurInd);
					m_iCurInd = i;

					return LT_WORD;
				}
				else
				{
					m_iCurInd = i;
					break;
				}
			}
		}
	}


	return LT_NONE;
}


//------------------------------------------------------------------------------------------
//тестовый вывод отпарсенных данных
void zTextParser::test_write_to_file(const std::wstring &file_name)
{
	zutl::MakeBackups(file_name, 2);
	std::ofstream file(file_name.c_str());

	std::string m_OutPool;
	int words_count = 0;

	std::string str;
	while(1)
	{
		LEX_TYPE type = get_next_word(str);
		if(type == LT_FILE_END)
		{
			if(words_count >= 4)
				file << m_OutPool << std::endl;
			break;
		}

		if(type == LT_SENTENCE_END)
		{
			if(words_count >= 4)
				file << m_OutPool << std::endl;
			words_count = 0;
			m_OutPool = "";
		}
		else
		if(type == LT_WORD)
		{
			m_OutPool += ((words_count == 0) ? "" : " ") + str;
			words_count++;
		}
		else
			(rel_assert(0));
	}

	file.close();
}

