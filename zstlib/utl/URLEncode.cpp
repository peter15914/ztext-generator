/*****************************************************************************
Module :     URLEncode.cpp
Notices:     Written 2002 by ChandraSekar Vuppalapati
Description: CPP URL Encoder
*****************************************************************************/


#include "stdafx.h"
//#include <math.h>
//#include <malloc.h>
//#include <memory.h>
//#include <new.h>
//#include <stdlib.h>
//#include <string.h>
//#include <WININET.H>

#include "URLEncode.h"

#define MAX_BUFFER_SIZE 4096

#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include <stdlib.h>
#include <crtdbg.h>
// HEX Values array
char hexVals[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
// UNSAFE String
std::string CURLEncode::csUnsafeString= "\"<>%\\^[]`+$,@:;/!#?=&";

// PURPOSE OF THIS FUNCTION IS TO CONVERT A GIVEN CHAR TO URL HEX FORM
std::string CURLEncode::convert(char val) 
{
	std::string csRet;
	csRet += "%";
	csRet += decToHex(val, 16);	
	return  csRet;
}

// THIS IS A HELPER FUNCTION.
// PURPOSE OF THIS FUNCTION IS TO GENERATE A HEX REPRESENTATION OF GIVEN CHARACTER
std::string CURLEncode::decToHex(char num, int radix)
{	
	int temp=0;	
	std::string cs_Tmp;
	int num_char = (int) num;
	
	// ISO-8859-1 
	// IF THE IF LOOP IS COMMENTED, THE CODE WILL FAIL TO GENERATE A 
	// PROPER URL ENCODE FOR THE CHARACTERS WHOSE RANGE IN 127-255(DECIMAL)
	if (num_char < 0)
		num_char = 256 + num_char;

	while (num_char >= radix)
    {
		temp = num_char % radix;
		num_char = num_char / radix;
		cs_Tmp = hexVals[temp];
    }

	cs_Tmp = hexVals[num_char] + cs_Tmp;

	if(cs_Tmp.length() < 2)
	{
		cs_Tmp = '0' + cs_Tmp;
	}
	
	return cs_Tmp;
}

// PURPOSE OF THIS FUNCTION IS TO CHECK TO SEE IF A CHAR IS URL UNSAFE.
// TRUE = UNSAFE, FALSE = SAFE
bool CURLEncode::isUnsafe(char compareChar)
{
	bool bcharfound = false;
	char tmpsafeChar;
	int m_strLen = 0;
	
	m_strLen = (int)csUnsafeString.length();
	for(int ichar_pos = 0; ichar_pos < m_strLen ;ichar_pos++)
	{
		tmpsafeChar = csUnsafeString[ichar_pos]; 
		if(tmpsafeChar == compareChar)
		{ 
			bcharfound = true;
			break;
		} 
	}
	int char_ascii_value = 0;
	//char_ascii_value = __toascii(compareChar);
	char_ascii_value = (int) compareChar;

	if(bcharfound == false &&  char_ascii_value > 32 && char_ascii_value < 123)
	{
		return false;
	}
	// found no unsafe chars, return false		
	else
	{
		return true;
	}
	
	return true;
}
// PURPOSE OF THIS FUNCTION IS TO CONVERT A STRING 
// TO URL ENCODE FORM.
std::string CURLEncode::_URLEncode(std::string pcs_Encode, bool custom)
{	
	int ichar_pos;
	std::string cs_Encode;
	std::string cs_Encoded;	
	int m_length;
	//int ascii_value;

	cs_Encode = pcs_Encode;
	m_length = (int)cs_Encode.length();
	
	for(ichar_pos = 0; ichar_pos < m_length; ichar_pos++)
	{
		char ch = cs_Encode[ichar_pos];
		if (ch < ' ') 
		{
			//ch = ch;
		}

		if(!isUnsafe(ch) || custom && (ch == ':' || ch == '/' || ch == '%'))
		{
			// Safe Character				
			cs_Encoded += ch;
		}
		else
		{
			// get Hex Value of the Character
			cs_Encoded += convert(ch);
		}
	}
	//
	return cs_Encoded;
}


std::string CURLEncode::URLEncode(std::string vData)
{
	return _URLEncode(vData, false);
}


//символы ':' и '/' эта функция считает нормальными
std::string CURLEncode::URLEncode_custom(std::string vData)
{
	return _URLEncode(vData, true);
}
