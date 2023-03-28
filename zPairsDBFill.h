#pragma once

class zPairsDB;
class zBooksDB;


class zPairsDBFill
{
public:
	zPairsDBFill();
	virtual ~zPairsDBFill();

	void add_pairs_from_books_db(zPairsDB *pairs_db, zBooksDB *books_db);
};
