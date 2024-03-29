
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
}

template <class DATATYPE, size_t BUFFER_LEN>
void BasePipe<DATATYPE, BUFFER_LEN>::Skip(size_t length){
	assert(m_len <= BUFFER_LEN);
	assert(m_pos < BUFFER_LEN);
	assert(length <= m_len);

	if (m_pos + length > BUFFER_LEN){
		m_pos = length - (BUFFER_LEN - m_pos);
	} else {
		m_pos += length;
		if (m_pos == BUFFER_LEN) m_pos = 0;
	}
	m_len -= length;
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

	m_mutex_read.lock();

	m_clean_flag = false;

	size_t read_length = 0; // Length that has been read
	while (read_length < read_min_length){
		m_mutex_rw.lock();

		if (m_len == 0){ // Block when has no data to read.
			if (m_end_flag){
				m_mutex_rw.unlock();
				break;
			}
			m_cond_var.wait(m_mutex_rw);
			if (m_clean_flag){
				m_pos = 0;
				m_len = 0;
				read_length = 0;
				m_mutex_rw.unlock();
				break;
			}
		}

		size_t r_length = read_max_length - read_length; // The remaining free length in the parameter buffer
		size_t length = r_length < m_len ? r_length : m_len;
		if (data)
			ReadTo(data + read_length, length);
		else
			Skip(length);
		read_length += length;

		m_mutex_rw.unlock();

		m_cond_var.signal();
	}

	m_mutex_read.unlock();
	return read_length;
}

template <class DATATYPE, size_t BUFFER_LEN>
size_t BasePipe<DATATYPE, BUFFER_LEN>::Write(DATATYPE *data, size_t write_length){

	m_mutex_write.lock();

	m_clean_flag = false;
	m_end_flag = false;

	size_t written_length = 0; // Length that has been written
	while (written_length < write_length){
		m_mutex_rw.lock();

		if (m_len == BUFFER_LEN){ // Block when has no free space to write.
			m_cond_var.wait(m_mutex_rw);
			if (m_clean_flag){
				m_pos = 0;
				m_len = 0;
				written_length = 0;
				m_mutex_rw.unlock();
				break;
			}
		}

		size_t r_length = write_length - written_length; // Remaining unwritten bytes
		size_t space = BUFFER_LEN - m_len; // Buffer's free bytes
		size_t length = r_length < space ? r_length : space;
		WriteIn(data + written_length, length);
		written_length += length;

		m_mutex_rw.unlock();

		m_cond_var.signal();
	}

	m_mutex_write.unlock();
	return written_length;
}

template <class DATATYPE, size_t BUFFER_LEN>
size_t BasePipe<DATATYPE, BUFFER_LEN>::Replace(DATATYPE *data, size_t write_length){

	m_mutex_write.lock();

	m_clean_flag = false;
	m_end_flag = false;

	m_pos = 0;
	m_len = 0;

	size_t written_length = 0; // Length that has been written
	while (written_length < write_length){
		m_mutex_rw.lock();

		if (m_len == BUFFER_LEN){ // Block when has no free space to write.
			m_cond_var.wait(m_mutex_rw);
			if (m_clean_flag){
				m_pos = 0;
				m_len = 0;
				written_length = 0;
				m_mutex_rw.unlock();
				break;
			}
		}

		size_t r_length = write_length - written_length; // Remaining unwritten bytes
		size_t space = BUFFER_LEN - m_len; // Buffer's free bytes
		size_t length = r_length < space ? r_length : space;
		WriteIn(data + written_length, length);
		written_length += length;

		m_mutex_rw.unlock();

		m_cond_var.signal();
	}

	m_mutex_rw.unlock();
	return written_length;
}

template <class DATATYPE, size_t BUFFER_LEN>
void BasePipe<DATATYPE, BUFFER_LEN>::NotifyEnd(){
	m_end_flag = true;
	m_cond_var.signal();
}

template <class DATATYPE, size_t BUFFER_LEN>
void BasePipe<DATATYPE, BUFFER_LEN>::Clear(){
	m_clean_flag = true;
	m_cond_var.signal();
}

template <class DATATYPE, size_t BUFFER_LEN>
size_t BasePipe<DATATYPE, BUFFER_LEN>::GetLength() const{
	return m_len;
}
