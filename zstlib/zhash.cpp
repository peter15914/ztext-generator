#include "stdafx.h"

#include <limits.h>

#include "zhash.h"


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//------------------------------------------------------------------------------------------
/*--- HashPJW-----------------------------------------------*
  (Адаптация PJW) обобщенного алгоритма хеширования
   Питера Веинбергер,   Основанного на версии Аллена Холуб.
   Принимает указатель на элемент данных, который нужно
   проиндексировать и    возвращает unsigned int
------------------------------------------------------------*/

#define BITS_IN_int     ( sizeof(int) * CHAR_BIT )
#define THREE_QUARTERS  ((int) ((BITS_IN_int * 3) / 4))
#define ONE_EIGHTH      ((int) (BITS_IN_int / 8))
#define HIGH_BITS       ( ~((unsigned int)(~0) >> ONE_EIGHTH ))

unsigned int zhash::GetHashPJW( const char * datum )
{
	unsigned int hash_value, i;

	for ( hash_value = 0; *datum; ++datum )
	{
		hash_value = ( hash_value << ONE_EIGHTH ) + *datum;
		if (( i = hash_value & HIGH_BITS ) != 0 )
			hash_value = (hash_value ^ (i >> THREE_QUARTERS)) & ~HIGH_BITS;
	}
	return ( hash_value );
}


//------------------------------------------------------------------------------------------
int zhash::GetHashNb(const char *sStr)
{
    int i = 0;
    int j = 1;
    char c;
	const char *buf = sStr;
    while ((c = *buf++))
		i += ((unsigned char)c) * (j++ + i % 999 );
	//
	int ret = i % 100000;
	return ret;
}


//------------------------------------------------------------------------------------------
unsigned zhash::GetHashAdler32(const char *sStr)
{
	unsigned low = 1;
	unsigned high = 0;
    unsigned char c;
	const char *buf = sStr;
    while ((c = *buf++))
	{
		low = (low + c) % 65521;
		high = (high + low) % 65521;
	}
	unsigned ret = high * 65536 + low;
	return ret;
}

