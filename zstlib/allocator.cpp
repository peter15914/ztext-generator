#include "stdafx.h"

#include "allocator.h"


#ifdef ZZZ_MEMORY_LEAK_DETECTION
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const char *StringPool::STATIC_EMPTY_STRING = "";

inline int RoundUp(DWORD cb, DWORD units)
{
	return ((cb + units - 1) / units) * units;
}

StringPool::StringPool()
: m_pchNext(NULL), m_pchLimit(NULL), m_phdrCur(NULL)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_dwGranularity = RoundUp(sizeof(HEADER) + MIN_CBCHUNK,
		si.dwAllocationGranularity);
}

LPSTR StringPool::AllocString_(const char* pszBegin, size_t _len)
{
	size_t _len2 = _len + 1;	//т.к. ноль в конце

	LPSTR psz = m_pchNext;
	if (m_pchNext + _len2 <= m_pchLimit) {
		m_pchNext += _len2;
		lstrcpynA(psz, pszBegin, _len2);
		return psz;
	}

	if (_len2 > MAX_CHARALLOC)
		return (rel_assert(0), 0);
	DWORD cbAlloc = RoundUp(_len2 * sizeof(char) + sizeof(HEADER),
		m_dwGranularity);
	BYTE* pbNext = reinterpret_cast<BYTE*>(
		VirtualAlloc(NULL, cbAlloc, MEM_COMMIT, PAGE_READWRITE));
		//malloc(cbAlloc));
	if (!pbNext)
		return (rel_assert(0), 0);

	m_pchLimit = reinterpret_cast<char*>(pbNext + cbAlloc);
	HEADER* phdrCur = reinterpret_cast<HEADER*>(pbNext);
	phdrCur->bar.m_phdrPrev = m_phdrCur;
	phdrCur->bar.m_cb = cbAlloc;
	m_phdrCur = phdrCur;
	m_pchNext = reinterpret_cast<char*>(phdrCur + 1);

	return AllocString_(pszBegin, _len);
}


StringPool::~StringPool()
{
	FreeAll();
}


void StringPool::FreeAll()
{
	HEADER* phdr = m_phdrCur;
	while (phdr) {
		HEADER hdr = *phdr;
		//VirtualFree(hdr.bar.m_phdrPrev, hdr.bar.m_cb, MEM_RELEASE);
		//phdr = hdr.bar.m_phdrPrev;

		HEADER *buf = hdr.bar.m_phdrPrev;
		VirtualFree(phdr, 0, MEM_RELEASE);
		phdr = buf;

		//HEADER *buf = hdr.bar.m_phdrPrev;
		//free(phdr);
		//phdr = buf;
	}

	m_pchNext = 0;
	m_pchLimit = 0;
	m_phdrCur = 0;
}
