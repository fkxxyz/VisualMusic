#include "thread.h"

#include <cassert>

template <int MAX_THREADS>
thread_pool_t<MAX_THREADS>::thread_pool_t(){

}

template <int MAX_THREADS>
thread_pool_t<MAX_THREADS>::thread_pool_t(int n_threads){
	bool result = start(n_threads);
	assert(result);
}

template <int MAX_THREADS>
thread_pool_t<MAX_THREADS>::~thread_pool_t(){
	bool result = stop();
	assert(result);
}

template <int MAX_THREADS>
bool thread_pool_t<MAX_THREADS>::start(int n_threads){
	m_n_threads = n_threads;
	m_n_free_threads = 0;
	m_stop_flag = false;
	for (int i = 0; i < m_n_threads; i++){
		assert(!m_threads[i].is_running());
		if (!m_threads[i].start(static_proc, this)){
			assert(0);
			return false;
		}
	}
	return true;
}

template <int MAX_THREADS>
bool thread_pool_t<MAX_THREADS>::stop(){
	if (!wait_all())
		return false;

	m_stop_flag = true;
	for (int i = 0; i < m_n_threads; i++)
		assert(m_threads[i].is_running());
	m_cond_has_tasks.signal();
	for (int i = 0; i < m_n_threads; i++)
		m_threads[i].join();
	return true;
}

template <int MAX_THREADS>
bool thread_pool_t<MAX_THREADS>::kill_all(){
	for (int i = 0; i < m_n_threads; i++){
		if (!m_threads[i].kill()){
			assert(0);
			return false;
		}
	}
}


template <int MAX_THREADS>
bool thread_pool_t<MAX_THREADS>::wait_all(){
	return m_event_all_finished.wait();
}

template <int MAX_THREADS>
bool thread_pool_t<MAX_THREADS>::is_working(){
	return !m_event_all_finished.is_setted();
}

template <int MAX_THREADS>
bool thread_pool_t<MAX_THREADS>::has_idle(){
	return m_n_free_threads > 0;
}


template <int MAX_THREADS>
void thread_pool_t<MAX_THREADS>::add_begin(){
	m_mutex_tasks.lock();
}

template <int MAX_THREADS>
void thread_pool_t<MAX_THREADS>::add(
		void *(*entry)(void *), void *param)
{
	struct task task;
	task.entry = entry;
	task.param = param;

	assert(m_mutex_tasks.is_locked());
	m_tasks.push(task);
}

template <int MAX_THREADS>
void thread_pool_t<MAX_THREADS>::add_end(){
	m_mutex_tasks.unlock();
	m_event_all_finished.reset();
	m_cond_has_tasks.signal();
}

template <int MAX_THREADS>
void *thread_pool_t<MAX_THREADS>::proc(){
	while (1){
		struct task task;

		// Get a task
		m_mutex_tasks.lock();
		{
			m_n_free_threads++;

			// Wait until there has tasks
			while (m_tasks.empty()){
				if (m_n_free_threads == m_n_threads)
					m_event_all_finished.set();
				m_cond_has_tasks.wait(m_mutex_tasks);
				if (m_stop_flag)
					goto t_end;
			}

			m_event_all_finished.reset();
			m_n_free_threads--;

			// Pop a task from the queue
			task = m_tasks.front();
			m_tasks.pop();
		}
		m_mutex_tasks.unlock();
		if (!m_tasks.empty())
			m_cond_has_tasks.signal();

		// Start the task
		task.entry(task.param);
	}
t_end:
	m_mutex_tasks.unlock();
	m_cond_has_tasks.signal();
	return nullptr;
}

template <int MAX_THREADS>
void *thread_pool_t<MAX_THREADS>::static_proc(void *pthis){
	return reinterpret_cast<thread_pool_t<MAX_THREADS> *>(pthis)->proc();
}



