#include "thread.h"

#ifdef linux
#include <pthread.h>
#include <cassert>
#include <sys/time.h>

inline void ms_to_abstime(struct timespec &abstime, unsigned long milliseconds){
	struct timeval now;
	gettimeofday(&now, nullptr);
	unsigned long sec = milliseconds / 1000;
	unsigned long msec = milliseconds % 1000;
	abstime.tv_sec += sec;
	abstime.tv_nsec += now.tv_usec * 1000 + static_cast<long>(msec) * 1000000;
	if (abstime.tv_nsec >= 1000000000){
		abstime.tv_sec++;
		abstime.tv_nsec -= 1000000000;
	}
}

inline thread_t::thread_t(){
	pthread_t *handle = reinterpret_cast<pthread_t *>(m_handle);
	*handle = 0;
}

inline thread_t::~thread_t(){
}

inline bool thread_t::start(void *(*thread_t_proc)(void *), void *param){
	pthread_t *handle = reinterpret_cast<pthread_t *>(m_handle);
	return pthread_create(handle, nullptr, thread_t_proc, param) == 0;
}

inline bool thread_t::join(void **exit_code){
	pthread_t *handle = reinterpret_cast<pthread_t *>(m_handle);
	return pthread_join(*handle, exit_code) == 0;
}

inline bool thread_t::timed_join(unsigned long milliseconds, void **exit_code){
	pthread_t *handle = reinterpret_cast<pthread_t *>(m_handle);
	struct timespec abstime;
	ms_to_abstime(abstime, milliseconds);
	return pthread_timedjoin_np(*handle, exit_code, &abstime) == 0;
}

inline bool thread_t::kill(){
	pthread_t *handle = reinterpret_cast<pthread_t *>(m_handle);
	return pthread_cancel(*handle) == 0;
}

inline bool thread_t::is_running() const {
	const pthread_t *handle = reinterpret_cast<const pthread_t *>(m_handle);
	if (*handle == 0) return false;
	int result = pthread_kill(*handle, 0);
	return result == 0;
}

inline void thread_t::exit(void *exit_code){
	pthread_exit(exit_code);
}



inline mutex_t::mutex_t(){
	pthread_mutex_t *handle = reinterpret_cast<pthread_mutex_t *>(m_handle);
	int result = pthread_mutex_init(handle, nullptr);
	assert(result == 0);
}

inline mutex_t::~mutex_t(){
	pthread_mutex_t *handle = reinterpret_cast<pthread_mutex_t *>(m_handle);
	int result = pthread_mutex_destroy(handle);
	assert(result == 0);
}

inline bool mutex_t::lock(){
	pthread_mutex_t *handle = reinterpret_cast<pthread_mutex_t *>(m_handle);
	return pthread_mutex_lock(handle) == 0;
}

inline bool mutex_t::timed_lock(unsigned long milliseconds){
	pthread_mutex_t *handle = reinterpret_cast<pthread_mutex_t *>(m_handle);
	struct timespec abstime;
	ms_to_abstime(abstime, milliseconds);
	return pthread_mutex_timedlock(handle, &abstime) == 0;
}

inline bool mutex_t::trylock(){
	pthread_mutex_t *handle = reinterpret_cast<pthread_mutex_t *>(m_handle);
	return pthread_mutex_trylock(handle) == 0;
}

inline bool mutex_t::is_locked(){
	pthread_mutex_t *handle = reinterpret_cast<pthread_mutex_t *>(m_handle);
	if (pthread_mutex_trylock(handle) == 0){
		pthread_mutex_unlock(handle);
		return false;
	} else
		return true;
}

inline bool mutex_t::unlock(){
	pthread_mutex_t *handle = reinterpret_cast<pthread_mutex_t *>(m_handle);
	return pthread_mutex_unlock(handle) == 0;
}


inline cond_t::cond_t(){
	pthread_cond_t *handle = reinterpret_cast<pthread_cond_t *>(m_handle);
	int result = pthread_cond_init(handle, nullptr);
	assert(result == 0);
}

inline cond_t::~cond_t(){
	pthread_cond_t *handle = reinterpret_cast<pthread_cond_t *>(m_handle);
	int result = pthread_cond_destroy(handle);
	assert(result == 0);
}

inline bool cond_t::wait(mutex_t &mutex){
	pthread_cond_t *handle = reinterpret_cast<pthread_cond_t *>(m_handle);
	return pthread_cond_wait(
				handle,
				reinterpret_cast<pthread_mutex_t *>(mutex.m_handle)
				) == 0;
}

inline bool cond_t::timed_wait(mutex_t &mutex, unsigned long milliseconds){
	pthread_cond_t *handle = reinterpret_cast<pthread_cond_t *>(m_handle);
	pthread_mutex_t *p_mutex = reinterpret_cast<pthread_mutex_t *>(mutex.m_handle);

	struct timespec abstime;
	ms_to_abstime(abstime, milliseconds);
	return pthread_cond_timedwait(handle, p_mutex, &abstime) == 0;
}

inline bool cond_t::signal(){
	pthread_cond_t *handle = reinterpret_cast<pthread_cond_t *>(m_handle);
	return pthread_cond_signal(handle) == 0;
}





inline semaphore_t::semaphore_t(int init_value, int max_value){
	assert(init_value >= 0);
	assert(max_value >= init_value);

	semaphore_t_arg &arg = *reinterpret_cast<semaphore_t_arg *>(m_handle);

	arg.value = init_value;
	arg.max_value = max_value;

	int result;
	result = pthread_mutex_init(&arg.mutex, nullptr);
	assert(result == 0);
	result = pthread_cond_init(&arg.cond, nullptr);
	assert(result == 0);
}

inline semaphore_t::~semaphore_t(){
	semaphore_t_arg &arg = *reinterpret_cast<semaphore_t_arg *>(m_handle);

	int result;
	result = pthread_cond_destroy(&arg.cond);
	assert(result == 0);
	result = pthread_mutex_destroy(&arg.mutex);
	assert(result == 0);
}

inline bool semaphore_t::wait(){
	semaphore_t_arg &arg = *reinterpret_cast<semaphore_t_arg *>(m_handle);

	pthread_mutex_lock(&arg.mutex);
	while (!arg.value){
		if (pthread_cond_wait(&arg.cond, &arg.mutex)){
			pthread_mutex_unlock(&arg.mutex);
			return false;
		}
	}
	arg.value -= 1;
	pthread_mutex_unlock(&arg.mutex);
	if (arg.value)
		pthread_cond_signal(&arg.cond);

	return true;
}

inline bool semaphore_t::timed_wait(unsigned long milliseconds){
	semaphore_t_arg &arg = *reinterpret_cast<semaphore_t_arg *>(m_handle);

	struct timespec abstime;
	ms_to_abstime(abstime, milliseconds);

	pthread_mutex_lock(&arg.mutex);
	while (!arg.value){
		if (pthread_cond_timedwait(&arg.cond, &arg.mutex, &abstime)){
			pthread_mutex_unlock(&arg.mutex);
			return false;
		}
	}
	arg.value -= 1;
	pthread_mutex_unlock(&arg.mutex);
	if (arg.value)
		pthread_cond_signal(&arg.cond);
	return true;
}

inline bool semaphore_t::try_wait(){
	semaphore_t_arg &arg = *reinterpret_cast<semaphore_t_arg *>(m_handle);

	if (arg.value == 0)
		return false;
	pthread_mutex_lock(&arg.mutex);
	arg.value -= 1;
	pthread_mutex_unlock(&arg.mutex);
	return true;
}

inline void semaphore_t::post(int value){
	semaphore_t_arg &arg = *reinterpret_cast<semaphore_t_arg *>(m_handle);

	pthread_mutex_lock(&arg.mutex);
	if (arg.value == arg.max_value){
		assert(0);
		goto l_end;
	}
	arg.value += value;
l_end:
	pthread_mutex_unlock(&arg.mutex);
	if (arg.value)
		pthread_cond_signal(&arg.cond);
}


inline int semaphore_t::get_value() const {
	const semaphore_t_arg &arg = *reinterpret_cast<const semaphore_t_arg *>(m_handle);
	return arg.value;
}

inline void semaphore_t::set_value(int value){
	semaphore_t_arg &arg = *reinterpret_cast<semaphore_t_arg *>(m_handle);
	assert(value >= 0 && value <= arg.max_value);

	if (arg.value == value)
		return;
	pthread_mutex_lock(&arg.mutex);
	arg.value = value;
	pthread_mutex_unlock(&arg.mutex);
	if (arg.value)
		pthread_cond_signal(&arg.cond);
}




inline event_t::event_t(bool manual_reset, bool initial_state){
	event_t_arg &arg = *reinterpret_cast<event_t_arg *>(m_handle);
	arg.setted = initial_state;
	arg.manual_reset = manual_reset;

	int result;
	result = pthread_mutex_init(&arg.mutex, nullptr);
	assert(result == 0);
	result = pthread_cond_init(&arg.cond, nullptr);
	assert(result == 0);
}

inline event_t::~event_t(){
	event_t_arg &arg = *reinterpret_cast<event_t_arg *>(m_handle);

	int result;
	result = pthread_cond_destroy(&arg.cond);
	assert(result == 0);
	result = pthread_mutex_destroy(&arg.mutex);
	assert(result == 0);
}

inline bool event_t::wait(){
	event_t_arg &arg = *reinterpret_cast<event_t_arg *>(m_handle);

	pthread_mutex_lock(&arg.mutex);
	while (!arg.setted){
		if (pthread_cond_wait(&arg.cond, &arg.mutex)){
			pthread_mutex_unlock(&arg.mutex);
			return false;
		}
	}
	if (!arg.manual_reset)
		arg.setted = false;
	pthread_mutex_unlock(&arg.mutex);
	pthread_cond_signal(&arg.cond);
	return true;
}

inline bool event_t::timed_wait(unsigned long milliseconds){
	event_t_arg &arg = *reinterpret_cast<event_t_arg *>(m_handle);

	struct timespec abstime;
	ms_to_abstime(abstime, milliseconds);

	pthread_mutex_lock(&arg.mutex);
	while (!arg.setted){
		if (pthread_cond_timedwait(&arg.cond, &arg.mutex, &abstime)){
			pthread_mutex_unlock(&arg.mutex);
			return false;
		}
	}
	if (!arg.manual_reset)
		arg.setted = false;
	pthread_mutex_unlock(&arg.mutex);
	pthread_cond_signal(&arg.cond);
	return true;
}

inline bool event_t::set(){
	event_t_arg &arg = *reinterpret_cast<event_t_arg *>(m_handle);

	pthread_mutex_lock(&arg.mutex);
	arg.setted = true;
	pthread_mutex_unlock(&arg.mutex);
	pthread_cond_signal(&arg.cond);
	return true;
}

inline bool event_t::reset(){
	event_t_arg &arg = *reinterpret_cast<event_t_arg *>(m_handle);

	pthread_mutex_lock(&arg.mutex);
	arg.setted = false;
	pthread_mutex_unlock(&arg.mutex);
	return true;
}

inline bool event_t::is_setted() const{
	const event_t_arg &arg = *reinterpret_cast<const event_t_arg *>(m_handle);
	return arg.setted;
}


#include <unistd.h>
inline int get_count_of_processor(){
	return static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
}

#endif


