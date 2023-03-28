#pragma once

template<typename T>
class zvec
{
	int m_size;
	int m_alloced;
	T *m_data;

	enum
	{
		MIN_ALLOC_ELEMENTS = 4,
	};

public:
	__forceinline void init()
	{
		#ifdef _DEBUG
			m_size = 0;
			m_alloced = 0;
			m_data = 0;
		#else
			memset(this, 0, sizeof(*this));
		#endif
	}

	__forceinline void release()
	{
		free(m_data);
		init();
	}

	__forceinline void clear()
	{
		m_size = 0;
	}

	__forceinline void resize(int new_size)
	{
		if(m_size != new_size)
		{
			reserve(new_size);
			m_size = new_size;
		}
	}

	__forceinline void reserve(int new_size)
	{
		if(m_alloced < new_size)
		{
			m_data = (T*)realloc(m_data, sizeof(T) * new_size);
			m_alloced = new_size;
		}
	}

	void push_back(T val)
	{
		if(m_size == m_alloced)
		{
			m_alloced = m_alloced ? m_alloced << 1 : MIN_ALLOC_ELEMENTS;
			m_data = (T*)realloc(m_data, sizeof(T) * m_alloced);
		}
		m_data[m_size++] = val;
	}

	__forceinline int size() { return m_size; }

	__forceinline T get_at(int ind) { return m_data[ind]; }
	__forceinline T &operator[](int ind) { return m_data[ind]; }
};
