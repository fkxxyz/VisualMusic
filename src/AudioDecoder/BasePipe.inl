
#include "stdafx.h"
#include "BasePipe.h"
#include <cassert>
#include <cstring>

template <class DATATYPE, size_t BUFFER_LEN>
inline BasePipe<DATATYPE, BUFFER_LEN>::BasePipe():
	m_pos(0),
	m_len(0),
	m_end_flag(false)
{
	pthread_mutex_init(&mutexCriticalSection, nullptr);
	pthread_cond_init(&condVar, nullptr);
	pthread_mutex_init(&mutexRead, nullptr);
	pthread_mutex_init(&mutexWrite, nullptr);
}


template <class DATATYPE, size_t BUFFER_LEN>
void BasePipe<DATATYPE, BUFFER_LEN>::ReadTo(DATATYPE *data, size_t length){
	assert(m_len <= BUFFER_LEN);
	assert(m_pos < BUFFER_LEN);
	assert(length <= m_len);

	if (m_pos + length > BUFFER_LEN){
		size_t end_part_size = BUFFER_LEN - m_pos;
		memcpy(data, m_data + m_pos, end_part_size * sizeof(DATATYPE));
		memcpy(data + end_part_size, m_data, (length - end_part_size) * sizeof(DATATYPE));
		m_pos = length - end_part_size;
	} else {
		memcpy(data, m_data + m_pos, length * sizeof(DATATYPE));
		m_pos += length;
		if (m_pos == BUFFER_LEN) m_pos = 0;
	}
	m_len -= length;
}

template <class DATATYPE, size_t BUFFER_LEN>
void BasePipe<DATATYPE, BUFFER_LEN>::WriteIn(DATATYPE *data, size_t length){
	assert(m_len <= BUFFER_LEN);
	assert(m_pos < BUFFER_LEN);
	assert(length <= BUFFER_LEN - m_len);

	size_t m_pos_end = (m_pos + m_len) % BUFFER_LEN;
	if (m_pos_end + length > BUFFER_LEN){
		size_t end_part_size = BUFFER_LEN - m_pos_end;
		memcpy(m_data + m_pos_end, data, end_part_size * sizeof(DATATYPE));
		memcpy(m_data, data + end_part_size, (length - end_part_size) * sizeof(DATATYPE));
	} else {
		memcpy(m_data + m_pos_end, data, length * sizeof(DATATYPE));
	}
	m_len += length;
}

template <class DATATYPE, size_t BUFFER_LEN>
size_t BasePipe<DATATYPE, BUFFER_LEN>::Read(DATATYPE *data, size_t read_min_length, size_t read_max_length){
	assert(read_min_length <= read_max_length);

	pthread_mutex_lock(&mutexRead);

	m_clean_flag = false;
	if (m_len == 0 && m_end_flag)
		return 0;

	size_t read_length = 0; // Length that has been read
	while (read_length < read_min_length){
		pthread_mutex_lock(&mutexCriticalSection);

		if (m_len == 0){ // Block when has no data to read.
			pthread_cond_wait(&condVar, &mutexCriticalSection);
			if (m_clean_flag){
				m_pos = 0;
				m_len = 0;
				read_length = 0;
				pthread_mutex_unlock(&mutexCriticalSection);
				break;
			}
		}

		size_t r_length = read_max_length - read_length; // The remaining free length in the parameter buffer
		size_t length = r_length < m_len ? r_length : m_len;
		ReadTo(data + read_length, length);
		read_length += length;

		pthread_mutex_unlock(&mutexCriticalSection);

		pthread_cond_signal(&condVar);
	}

	pthread_mutex_unlock(&mutexRead);
	return read_length;
}

template <class DATATYPE, size_t BUFFER_LEN>
size_t BasePipe<DATATYPE, BUFFER_LEN>::Write(DATATYPE *data, size_t write_length){

	pthread_mutex_lock(&mutexWrite);

	m_clean_flag = false;
	m_end_flag = false;

	size_t written_length = 0; // Length that has been written
	while (written_length < write_length){
		pthread_mutex_lock(&mutexCriticalSection);

		if (m_len == BUFFER_LEN){ // Block when has no free space to write.
			pthread_cond_wait(&condVar, &mutexCriticalSection);
			if (m_clean_flag){
				m_pos = 0;
				m_len = 0;
				written_length = 0;
				pthread_mutex_unlock(&mutexCriticalSection);
				break;
			}
		}

		size_t r_length = write_length - written_length; // Remaining unwritten bytes
		size_t space = BUFFER_LEN - m_len; // Buffer's free bytes
		size_t length = r_length < space ? r_length : space;
		WriteIn(data + written_length, length);
		written_length += length;

		pthread_mutex_unlock(&mutexCriticalSection);

		pthread_cond_signal(&condVar);
	}

	pthread_mutex_unlock(&mutexWrite);
	return written_length;
}

template <class DATATYPE, size_t BUFFER_LEN>
void BasePipe<DATATYPE, BUFFER_LEN>::NotifyEnd(){
	m_end_flag = true;
}

template <class DATATYPE, size_t BUFFER_LEN>
void BasePipe<DATATYPE, BUFFER_LEN>::Clear(){
	m_clean_flag = true;

	pthread_cond_signal(&condVar);
}

template <class DATATYPE, size_t BUFFER_LEN>
size_t BasePipe<DATATYPE, BUFFER_LEN>::GetLength() const{
	return m_len;
}
