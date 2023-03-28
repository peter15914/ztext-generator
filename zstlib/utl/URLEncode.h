/*****************************************************************************
Module :     URLEncode.H
Notices:     Written 2002 by ChandraSekar Vuppalapati
Description: H URL Encoder
*****************************************************************************/
#ifndef __CURLENCODE_H_
#define __CURLENCODE_H_


//---
class CURLEncode
{
private:
	static std::string csUnsafeString;
	std::string decToHex(char num, int radix);
	bool isUnsafe(char compareChar);
	std::string convert(char val);

	std::string _URLEncode(std::string vData, bool custom);

public:
	CURLEncode() { };
	virtual ~CURLEncode() { };
	//
	std::string URLEncode(std::string vData);

	//символы ':' и '/' эта функция считает нормальными
	std::string URLEncode_custom(std::string vData);
};

#endif //__CURLENCODE_H_