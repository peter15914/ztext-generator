#pragma once
/*/----------------------------------------------------------------------------
	@namespace zhash
	@desc


	2008May15 created. zm
-----------------------------------------------------------------------------*/


//------------------------------------------------------------------------------
namespace zhash
{
	unsigned int GetHashPJW( const char * datum );
	int GetHashNb(const char *sStr);
	unsigned GetHashAdler32(const char *sStr);
	//inline int GetHash(const char *sStr) { return GetHashPJW(sStr); };
	//inline int GetHash(const char *sStr) { return (GetHashNb(sStr); };
	inline int GetHash(const char *sStr) { return (GetHashNb(sStr) + GetHashAdler32(sStr)); };
};

