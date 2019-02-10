#pragma once

#include <pthread.h>
#include <deque>

template<typename T>
class BlockingQueue {
	std::deque<T> deque{};
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	bool cancelled = false;
public:
	~BlockingQueue();
	T take();
	T &front();
	T &back();
	void put(const T&);
	void put(T&&);
	template< class... Args >
	void emplace_back(Args&&... args);
	void clear();
	void cancel();
};

#define run_and_notify(block) \
pthread_mutex_lock(&this->lock); \
block \
pthread_cond_signal(&this->cond); \
pthread_mutex_unlock(&this->lock);

template<typename T>
BlockingQueue<T>::~BlockingQueue() {
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);
}

template<typename T>
T BlockingQueue<T>::take() {
	pthread_mutex_lock(&lock);
	cancelled = false;
	while (deque.empty() && !cancelled)
		pthread_cond_wait(&cond, &lock);
	if (cancelled)
		pthread_exit(nullptr);
	T ret(std::move(deque.front()));
	deque.pop_front();
	pthread_mutex_unlock(&lock);
	return ret;
}

template<typename T>
void BlockingQueue<T>::put(const T &s) {
	run_and_notify({ deque.push_back(s); })
}

template<typename T>
void BlockingQueue<T>::put(T &&s) {
	run_and_notify({ deque.push_back(std::move(s)); })
}

template<typename T>
T &BlockingQueue<T>::front() {
	pthread_mutex_lock(&lock);
	auto &ret = deque.front();
	pthread_mutex_unlock(&lock);
	return ret;
}

template<typename T>
T &BlockingQueue<T>::back() {
	pthread_mutex_lock(&lock);
	auto &ret = deque.back();
	pthread_mutex_unlock(&lock);
	return ret;
}

template<typename T>
template<class... Args>
void BlockingQueue<T>::emplace_back(Args &&... args) {
	run_and_notify({ deque.emplace_back(std::forward<Args>(args)...); })
}

template<typename T>
void BlockingQueue<T>::clear() {
	pthread_mutex_lock(&lock);
	std::deque<T> t;
	deque.swap(t);
	pthread_mutex_unlock(&lock);
}

template<typename T>
void BlockingQueue<T>::cancel() {
	run_and_notify({
		cancelled = true;
		std::deque<T> t;
		deque.swap(t);
	})
}
