#pragma once


//<word3, count>
typedef std::map<int, int> zWords3Map;


//------------------------------------------------------------------------------------------
class zWordPair
{
	int m_word1;
	int m_word2;

	//количество раз, которое пара встречается перой в предложении
	int m_first_cnt;

	zWords3Map m_Words3Map;

public:
	zWordPair();
	virtual ~zWordPair();

	inline int get_word1() const {return m_word1; };
	inline int get_word2() const {return m_word2; };

	inline void set_words(int word1, int word2)
	{
		m_word1 = word1;
		m_word2 = word2;
	}

	//add word3 (or increase it's count)
	//return current word3 frequency
	int add_word3(int word3);

	void inc_first_cnt() { ++m_first_cnt; }

	//возвращает сколько раз эта пара встречалась (для отладки)
	//если будешь использовать эту функцию в реале, то сделай кеширование значения
	int get_cnt_4_test();

	//величина мапы с третьими словами (для отладки)
	int get_word3_map_size_4_test() { return (int)m_Words3Map.size(); };
};


//------------------------------------------------------------------------------------------
namespace zpairsdb
{
	struct cmp_pairs
	{
	   bool operator()(const zWordPair *a, const zWordPair *b) const
	   {
		   return (a->get_word1() < b->get_word1()) || (a->get_word1() == b->get_word1()) && (a->get_word2() < b->get_word2());
	   }
	};
};

