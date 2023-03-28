#include "stdafx.h"

#include "zPairsDBFill.h"
#include "zPairsDB.h"
#include "zBooksDB.h"


//------------------------------------------------------------------------------------------
zPairsDBFill::zPairsDBFill()
{
}


//------------------------------------------------------------------------------------------
zPairsDBFill::~zPairsDBFill()
{
}


//------------------------------------------------------------------------------------------
void zPairsDBFill::add_pairs_from_books_db(zPairsDB *pairs_db, zBooksDB *books_db)
{
	//for(int i = 0; i < books_db->get_books_cnt(); i++)
	for(int i = 0; i < 50; i++)
	{
		int book_id = books_db->get_id_by_num(i);
		zBookInfo *book = books_db->get_book_by_id(book_id);

		const std::vector<int> &words = book->get_words();
		int cnt = (int)words.size();
		if(cnt < 3)
			continue;

		zdebug::log()->Log("book: " + book->get_stat_str());

		int w0 = BI_SENT_END;
		int w1 = BI_SENT_END;
		int w2 = words[0];
		int w3 = words[1];

		for(int jj = 2; jj < cnt; jj++)
		{
			w0 = w1;
			w1 = w2;
			w2 = w3;
			w3 = words[jj];

			if(w1 != BI_SENT_END && w2 != BI_SENT_END)
				pairs_db->add_tripple(w1, w2, w3, w0 == BI_SENT_END);
		}
	}
}

