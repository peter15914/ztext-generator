#pragma once

#include "zWordsDB.h"


//------------------------------------------------------------------------------------------
class zStatProcs : public boost::noncopyable
{
	zWordsDB m_words_db;

	std::vector<unsigned int> cnt_vec;
	std::vector<unsigned int> w3count_vec;
	std::vector<int> w3result_vec;

	std::vector<__int64> pairs_vec;
	std::vector<bool> m_is_one_letter;
	std::vector<unsigned int> m_is_first_pair_;
	std::vector<unsigned short> m_last_word;

	//для _add_pairs_cnt
	__int64 m_val_min, m_val_max;

public:
	zStatProcs();
	virtual ~zStatProcs() {};

	void create_pairs_stat_file();
	bool output_pairs_file(std::set<__int64> &pairs_set, std::wstring file_name);
	bool merge_pairs_files(std::wstring file_name_1, std::wstring file_name_2, std::wstring file_name_out);

	//считаем статистику для пар из диапазона [ind_first, ind_last
	//эта та самая, долгая функция, кот. создает файлы типа 370000000_430000000.stat
	void check_pairs_stat(__int64 ind_first, __int64 ind_last);

	//
	void test_binary_search();
	void test_output_one_letter_sent();

	//подсчитать длины подмассивов, на которые разобъем итоговый массив
	void test_best50kk_subarr_len();

	//конвертнуть бинарный формат cnt_vec в текстовый
	//ind_first и ind_last указываются руками
	bool save_to_text_format(std::wstring pairs_file_name, __int64 ind_first, __int64 ind_last,
								std::wstring file_name1, std::wstring file_name2);

	//
	bool test_find_rare_pairs(std::wstring file_name);

	//составляет список пар, кот. являются первыми в предложениях
	bool find_first_pairs();

	//подсчитывает, сколько каждое слово бывает последним в предложениях
	bool find_last_words();

	//вывести окончательный, хороший файл с парами
	//содержит только пары, встретившиеся чаще 6 раз (чтоб было около 50 млн.)
	bool output_best_pairs_file();

	//подсчитать, сколько после каждой пары может идти третьих слов
	bool calc_pairs_word3();
	//та же функция, но записывает их в массив
	bool calc_pairs_word3_fill(int ind_first, int ind_last, unsigned int arr_size);

	//
	inline static __int64 get_pair_from_words(int word1, int word2)
	{
		__int64 buf = word1;
		buf = buf << 32;
		buf += word2;
		return buf;
	}

	//не очень быстрая версия, т.к. для проверки
	inline static void get_words_from_pair(__int64 pair_val, int &word1, int &word2)
	{
		__int64 w1_64 = pair_val >> 32;
		__int64 w2_64 = pair_val - (w1_64 << 32);
		rel_assert(w1_64 <= INT_MAX && w2_64 <= INT_MAX && w1_64 >= INT_MIN && w2_64 >= INT_MIN);

		word1 = (int)w1_64;
		word2 = (int)w2_64;
	}

private:
	//читает пары в pairs_vec, возвращает размер
	int _read_pairs(std::wstring file_name, __int64 ind_first, __int64 ind_last);

	//вернуть индекс пары в массиве pairs_vec, использует бинарный поиск
	//INT_MAX, если отсутствует
	int _get_pair_ind(__int64 pair_val);

	//дозагрузить всё, что необходимо
	void _load_all_needed();

	//
	void _fill_is_one_letter();

	//увеличивает количество пар для статистики в cnt_vec
	inline void _add_pairs_cnt(const int &word1, const int &word2);

	//ставит и снимает с паузы по Ctrl+Shift+RWin
	void _check_if_paused();

	//
	void _get_sent(const std::vector<int> &words, int first_ind, int last_ind, std::string &ret_str);
};

