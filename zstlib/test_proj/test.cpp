#include "stdafx.h"

#include "..\zstr.h"


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//------------------------------------------------------------------------------------------
void testStrings()
{
	std::string str;
	str = "dfasdvgasgghg";
	printf("%s\n", str.c_str());
	//
	zstr::ToUpper(str);
	printf("%s\n", str.c_str());
}


//------------------------------------------------------------------------------------------
void test_w_to_s_to_w()
{
	std::string s = "C:/work/111.dll";

	std::wstring ws = zstr::s_to_w(s);

	std::wstring ws2 = L"C:/work/111.exe";
	std::string s2 = zstr::w_to_s(ws2);

	int ii = 0;
}


//------------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	//testStrings();

	test_w_to_s_to_w();
	//
	return 0;
}

