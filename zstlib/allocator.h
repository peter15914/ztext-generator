#pragma once


#include <boost/noncopyable.hpp>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

class Allocator : public boost::noncopyable
{
public:
	Allocator()
		: m_BlockSize(0), m_BlockCount(0), m_Head(0)
	{
	}

	Allocator(size_t BlockSize, size_t BlockCount = 32)
		: m_BlockSize(BlockSize), m_BlockCount(BlockCount), m_Head(0)
	{
		m_BlockSize = max(sizeof(void*), m_BlockSize);
		m_BlockCount = max(1, m_BlockCount);
	}

	~Allocator()
	{
		FreeAll();
	}

	void SetParams(size_t BlockSize, size_t BlockCount)
	{
		_ass(!m_Head);
		m_BlockSize = BlockSize;
		m_BlockCount = BlockCount;
		m_BlockSize = max(sizeof(void*), m_BlockSize);
		m_BlockCount = max(1, m_BlockCount);
	}

	void* GetMemory()
	{
		if (!m_Head)
			m_Head = NewBuffer();
		void* Result = m_Head;
		m_Head = *(char**)m_Head;
		return Result;
	}

	void FreeMemory(void* Ptr)
	{
		//ASSERT(Ptr);
		*(char**)Ptr = m_Head;
		m_Head = (char*) Ptr;
	}

	void FreeAll()
	{
		for (size_t Index = 0; Index < m_Buffers.size(); Index++)
			delete[] m_Buffers[Index];
		m_Buffers.clear();
		m_Head = 0;
	}

	size_t BlockSize() const
	{
		return m_BlockSize;
	}

	size_t BlockCount() const
	{
		return m_BlockCount;
	}

protected:
	char* NewBuffer()
	{
		size_t TotalSize = m_BlockSize * m_BlockCount;
		char* Buffer = new char [TotalSize];
		for (size_t Index = 0; Index < TotalSize - m_BlockSize; Index += m_BlockSize)
			(char*&) Buffer[Index] = Buffer + Index + m_BlockSize;
		(char*&) Buffer[TotalSize - m_BlockSize] = 0;
		m_Buffers.push_back(Buffer);
		return Buffer;
	}

protected:
	size_t m_BlockSize;
	size_t m_BlockCount;
	char* m_Head;
	std::vector<char*> m_Buffers;
};


class StringPool
{
public:
	StringPool();
	~StringPool();

	char *AllocString_(const char* pszBegin, size_t _len);
	char *AllocString_(const std::string str)
	{
		return AllocString_(str.c_str(), str.length());
	}
	char *AllocString_(const std::string str, size_t nBeg, size_t _len)
	{
		size_t cur_len = str.length();
		if(nBeg >= cur_len)
			return (_ass(0), 0);

		if(nBeg + _len > cur_len)
			_len = cur_len - nBeg;

		return AllocString_(str.c_str() + nBeg, _len);
	}

	void FreeAll();

	static const char *STATIC_EMPTY_STRING;

private:
	union HEADER {
		struct foo {
			HEADER* m_phdrPrev;
			size_t  m_cb;
		} bar;
		char alignment;
	};
	enum { MIN_CBCHUNK = 32000,
		MAX_CHARALLOC = 1024*1024 };

private:
	char*  m_pchNext;   // first available byte
	char*  m_pchLimit;  // one past last available byte
	HEADER* m_phdrCur;   // current block
	unsigned long   m_dwGranularity;
};

