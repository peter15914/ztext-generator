#pragma once

#include "zWordsDBFast.h"
#include "zWordsDB.h"


//------------------------------------------------------------------------------------------
struct Z_W3DB_FILE
{
	std::string fname;
	unsigned int m_size;	//количество элементов, кот. хранится в файле
	unsigned int m_shift;	//сколько значений до этого файла (чтоб не пробегать кажды раз сначала)
	FILE *pFile;

	int m_DbgUsage;	//сколько раз обращались к файлу

	Z_W3DB_FILE()
	{
		pFile = 0;
		m_size = 0;
		m_DbgUsage = 0;
		m_shift = 0;
	}
};


//------------------------------------------------------------------------------------------
class zGenProcs
{
	//не храним, чтоб больше в оперативку влазило
	//zWordsDB m_words_db;
	//zWordsDBFast m_words_db_fast;

	int m_words_db_size;	//размер базы слов, одновременно - флаг, что уже всё загружено

	std::vector<unsigned int> m_w3count_vec;
	std::vector<__int64> m_pairs_vec;				//огроменный массив на 50kk пар для поиска бинарным поиском
	//std::vector<unsigned int> m_first_pair_vec;	//убран, чтобы не занимать память

	std::vector<__int64> m_first_pairs_gen;			//заранее сгенеренные первые пары
	int m_first_pairs_used;

	//std::vector<int> m_w3db_cache;					//классный ускоряющий кэш
	std::vector<unsigned char> m_w3db_cache_;					//классный ускоряющий кэш
	std::vector<unsigned char> m_w3db_cache_2;					//добавка (один массив подряд не выделяется)

	//буфер для чтения из w3db3-файлов
	std::vector<unsigned char> m_w3db_fread_buf_;
	//нулевой элемент m_w3db_fread_buf  - это на самом деле m_w3db_fread_cur_ind-ый элемент (глобальный)
	unsigned int m_w3db_fread_cur_ind_;
	unsigned int m_w3db_fread_ind_last;		//иногда ведь не целиком считываем

	//слова, перед которыми ставят запятую
	std::set<int> m_comma_words;

	//слова, которые не могут быть последними
	std::set<int> m_not_last_words;

	//однобуквенные слова
	std::set<int> m_one_letter_words;

	int m_pairs_cnt;

	std::vector<Z_W3DB_FILE> m_w3db_files;
	__int64 m_w3db_size;

	//сколько раз читали мимо cache
	int m_dbg_no_cache_read_cnt;
	//сколько была особая проблема при генерации предложения
	int m_dbg_sent_gen_problem_cnt;
	//сколько раз приходилось получать новое слово из-за проблем типа 3-х однобуквенных подряд
	int m_dbg_wrod_reget_problem_cnt;

	//для особого режима генерации с ключевиками
	//если m_first_pair != -1, то каждое предложение должно начинаться с этой пары
	__int64 m_first_pair_4key;
	//задаются в функции gen_with_first_pair, там есть описание
	std::string m_prefix_4key, m_postfix_4key;

public:
	zGenProcs();
	virtual ~zGenProcs();


	//конвертит предложение в строку
	void gen_sentence_text(std::string &ret, const std::vector<int> &sent, zWordsDBFast &words_db_fast);

	//выдает рандомные предложения в файл
	void gen_sentences(const wchar_t *file_name, int sent_cnt, int sent_len_min, int sent_len_max);

	//тестово выдает несколько рандомных предложений в лог
	void test_gen_sentences();

	//тестово показать статистику первых пар
	void show_first_pairs_stat();

	//выводит в лог частоту используемости каждого w3db-файла
	void show_w3db_usage();
	void clear_w3db_usage();

	//тесты скорости некоторых функций
	int test_performance();

	//функция конвертирует w3db-базу в базу, которая хранит 3 байта на каждое третье слово
	void convert_w3db_to_3bytes();

	//для режима генерации с ключевиками
	//возвращает false, если не удалось найти пару для word1 и word2
	//word1 и word2 - слова, из которх будет состоять первая пара
	//prefix и postfix - строки, которые будут писаться перед и после пары <word1, word2>, они не влияют
	//на генерацию, а только на вывод в файл
	bool gen_with_first_pair(std::string word1, std::string word2, std::string prefix, std::string postfix);

private:
	//загрузить всё, что необходимо (массивы, базы)
	//если уже загружено, то ничего не делает
	//возвращает false, если не получилось, надо прекращать генерацию
	bool _load_all_needed();

	//загрузить данные о w3db-файлах
	void _load_w3db_files();

	//создать кэш для снижения количества обращений к винту
	void _create_w3db_cache();

	//создать заранее сгенеренные первые пары. читает файл first_pairs.dat, а потом выгружает.
	//генерит в m_first_pairs_gen некоторое количество первых пар
	void _generate_first_pairs();

	//загружаем необходимые данные из zWordsDB и zWordsDBFast (не храним эти базы, что больше памяти было)
	//если передаем word1_4key и word2_4key, то устанавливаем m_first_pair_4key
	void _load_data_from_words_db(const char *word1_4key = 0, const char *word2_4key = 0);

	//вернуть индекс пары в массиве pairs_vec, использует бинарный поиск
	//INT_MAX, если отсутствует
	int _get_pair_ind(__int64 pair_val);

	//генерирует предложение в массив
	void _gen_sentence(std::vector<int> &sent, int sent_len_min, int sent_len_max);

	//выдает "случайную" первую пару из ранее нагенеренного m_first_pairs_gen
	__int64 _get_rand_first_pair_fast();

	//выдает случайное word3, выдает BI_SENT_END, если больше нет
	//каждый раз обращается к винту (new: теперь есть кеш)
	int _get_rand_word3(__int64 pair);

	//функция устарела, не используем больше
	//достает word3 из огроменного файломассива
	//без всяких буферов, так что медленно и печально для винта
	//int _get_word3(unsigned int w3_ind);

	//достает word3 из огроменного файломассива
	int _get_word3_ex(Z_W3DB_FILE *b, unsigned int w3_ind);
	//для случая перегенерации кэша
	int _get_word3_ex(unsigned int w3_ind);

	//является ли слово однобуквенным
	bool _is_one_letter_word(int id) { return m_one_letter_words.find(id) != m_one_letter_words.end(); }
};
