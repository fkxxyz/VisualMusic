#pragma once
#include "stdafx.h"
#include <cstddef>
#include <pthread.h>

template <class DATATYPE, size_t BUFFER_LEN>
class BasePipe {
public:
	BasePipe();

	virtual size_t Read(DATATYPE *data, size_t read_min_length, size_t read_max_length);
	virtual size_t Write(DATATYPE *data, size_t write_length);
	virtual void Clear();
	virtual void NotifyEnd();
	virtual size_t GetLength() const;

protected:
	DATATYPE m_data[BUFFER_LEN];
	size_t m_pos, m_len;

	pthread_mutex_t mutexRead, mutexWrite;
	pthread_cond_t condVar;
	pthread_mutex_t mutexCriticalSection;

	bool m_clean_flag;
	bool m_end_flag;

	void Skip(size_t length);
	void ReadTo(DATATYPE *data, size_t length);
	void WriteIn(DATATYPE *data, size_t length);
};

#include "BasePipe.inl"

