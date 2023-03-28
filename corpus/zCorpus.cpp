#include "stdafx.h"

#include "zCorpus.h"

using namespace std;


//------------------------------------------------------------------------------------------
zCorpus::zCorpus()
{
}


//------------------------------------------------------------------------------------------
zCorpus::~zCorpus()
{
}


//------------------------------------------------------------------------------------------
//добавить данные из файла в корпус
void zCorpus::add_DataToCorpus(std::string file_name)
{
	ifstream file(file_name.c_str());
	string line;

	string s1, s2, s3;
	s1 = s2 = s3 = "$";

	while(file && getline(file, line))
	{
		string ss;
		for(size_t i = 0; i <= line.length(); i++)
		{
			char c = (i < line.length()) ? line[i] : ' ';
			if(c >= 'а' && c <= 'я' || c >= 'А' && c <= 'Я')
				ss += c;
			else		//todo разделителями слов считать только пробельные и знаки препинания
			{
				if(!ss.empty())
				{
					s1 = s2;
					s2 = s3;
					s3 = ss;
					ss = "";

					string key = _get_key(s1, s2);

					zStatWords *statWords = 0;

					std::map<std::string, size_t>::iterator it = m_Data.find(key);
					if(it == m_Data.end())
					{
						size_t sz = m_StatWords.size();
						m_StatWords.resize(sz+1);
						m_Data[key] = sz;
						statWords = &m_StatWords[sz];
					}
					else
						statWords = &m_StatWords[it->second];

					statWords->add_word(s3);

					if(c == '.')
					{
						s1 = s2 = s3 = "$";
					}
				}
			}
		}
	}

	file.close();
	
}


//------------------------------------------------------------------------------------------
//для теста нагенерим немного текста
void zCorpus::test_GenSomeText(std::string file_name)
{
	ofstream file_out(file_name.c_str());

	//100 предложений
	for(int i = 0; i < 100; i++)
	{
		//составляем 1 предложение
		string str;

		//генерим, пока есть слова, но не более 20 слов в предложении
		string s1, s2, s3;
		s1 = s2 = s3 = "$";

		for(int j = 0; j < 100; j++)
		{
			zStatWords &vec = get_WordsVec(s1, s2);

			//рандомное слово
			size_t sz = vec.size();
			if(sz == 0)
				break;

			size_t rand_ind = size_t(rand() % sz);
			s3 = vec.get_word_temp(rand_ind);

			str += s3;
			str += " ";

			s1 = s2;
			s2 = s3;
		}

		str += ".";

		file_out << str << endl;
	}

	file_out.close();
}


//------------------------------------------------------------------------------------------
string zCorpus::_get_key(const string &s1, const string &s2)
{
	return s1 + "#" + s2;
}


//------------------------------------------------------------------------------------------
zStatWords &zCorpus::get_WordsVec(const string &s1, const string &s2)
{
	static zStatWords buf;

	string key = _get_key(s1, s2);
	std::map<std::string, size_t>::iterator it = m_Data.find(key);

	if(it == m_Data.end())
		return buf;

	return m_StatWords[it->second];
}
