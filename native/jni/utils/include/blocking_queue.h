#pragma once

#include <pthread.h>
#include <deque>
#include <misc.h>

template<typename T>
class blocking_queue {
	std::deque<T> deque{};
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	bool cancelled = false;
public:
	~blocking_queue();
	T take();
	T &front() const;
	T &back() const;
	void push(const T&);
	void push(T&&);
	template< class... Args >
	void emplace(Args &&... args);
	void clear();
	void cancel();
};

#define run_and_notify(block) \
mutex_guard g(this->lock); \
block \
pthread_cond_signal(&this->cond);

template<typename T>
blocking_queue<T>::~blocking_queue() {
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);
}

template<typename T>
T blocking_queue<T>::take() {
	mutex_guard g(lock);
	cancelled = false;
	while (deque.empty() && !cancelled)
		pthread_cond_wait(&cond, &lock);
	if (cancelled)
		pthread_exit(nullptr);
	T ret(std::move(deque.front()));
	deque.pop_front();
	return ret;
}

template<typename T>
void blocking_queue<T>::push(const T &s) {
	run_and_notify({ deque.push_back(s); })
}

template<typename T>
void blocking_queue<T>::push(T &&s) {
	run_and_notify({ deque.push_back(std::move(s)); })
}

template<typename T>
T &blocking_queue<T>::front() const {
	mutex_guard g(lock);
	return deque.front();
}

template<typename T>
T &blocking_queue<T>::back() const {
	mutex_guard g(lock);
	return deque.back();
}

template<typename T>
template<class... Args>
void blocking_queue<T>::emplace(Args &&... args) {
	run_and_notify({ deque.emplace_back(std::forward<Args>(args)...); })
}

template<typename T>
void blocking_queue<T>::clear() {
	mutex_guard g(lock);
	std::deque<T> t;
	deque.swap(t);
}

template<typename T>
void blocking_queue<T>::cancel() {
	run_and_notify({
		cancelled = true;
		std::deque<T> t;
		deque.swap(t);
	})
}

#undef run_and_notify
