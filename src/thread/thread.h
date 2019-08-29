#pragma once
#include "stdafx.h"

#include "threadtypes.h"

class thread_t {
public:
	thread_t();
	virtual ~thread_t();

	bool start(void *(*thread_proc)(void *), void *param);
	bool join(void **exit_code = nullptr);
	bool timed_join(unsigned long milliseconds, void **exit_code = nullptr);
	bool kill();
	bool is_running() const;

	static void exit(void *exit_code);

protected:

	char m_handle[SIZEOF_THREAD_T];
};

class thread_proc_t : public thread_t {
public:
	bool start(){
		return thread_t::start(thread_entry, this);
	}

protected:
	virtual void *proc() = 0;
	static void *thread_entry(void *pthis){
		return reinterpret_cast<thread_proc_t *>(pthis)->proc();
	}
};

class mutex_t {
public:
	mutex_t();
	virtual ~mutex_t();

	bool lock();
	bool timed_lock(unsigned long milliseconds);
	bool trylock();
	bool is_locked();
	bool unlock();

protected:
	char m_handle[SIZEOF_MUTEX_T];
	friend class cond_t;
};

class cond_t {
public:
	cond_t();
	virtual ~cond_t();

	bool wait(mutex_t &mutex);
	bool timed_wait(mutex_t &mutex, unsigned long milliseconds);
	bool signal();

protected:
	char m_handle[SIZEOF_COND_T];
};

class semaphore_t {
public:
	semaphore_t(int init_value, int max_value);
	virtual ~semaphore_t();

	bool wait();
	bool timed_wait(unsigned long milliseconds);
	bool try_wait();
	void post(int value = 1);
	int get_value() const;
	void set_value(int value);

protected:
	char m_handle[SIZEOF_SEMAPHORE_T];
};

class event_t {
public:
	event_t(bool manual_reset = true, bool initial_state = false);
	virtual ~event_t();

	bool wait();
	bool timed_wait(unsigned long milliseconds);
	bool set();
	bool reset();
	bool is_setted() const;

protected:
	char m_handle[SIZEOF_EVENT_T];
};


#include <list>
#include <queue>

template <int MAX_THREADS>
class thread_pool_t {
public:
	thread_pool_t();
	thread_pool_t(int n_threads);
	virtual ~thread_pool_t();

	bool start(int n_threads);
	bool stop();
	bool kill_all();

	bool wait_all();
	bool is_working();
	bool has_idle();

	void add_begin();
	void add(void *(*entry)(void *), void *param);
	void add_end();

protected:
	thread_t m_threads[MAX_THREADS];
	int m_n_threads;

	struct task {
		void *(*entry)(void *);
		void *param;
	};
	queue<struct task, list<struct task>> m_tasks;
	int m_n_free_threads;
	mutex_t m_mutex_tasks;
	cond_t m_cond_has_tasks;
	event_t m_event_all_finished;

	bool m_stop_flag;

	void *proc();
	static void *static_proc(void *);
};

int get_count_of_processor();


#include "thread/thread_common.inl"
#include "thread/thread_linux.inl"
#include "thread/thread_windows.inl"


