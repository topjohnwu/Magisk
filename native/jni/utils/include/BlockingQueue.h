#pragma once

#include <pthread.h>
#include <deque>

template<typename T>
class BlockingQueue {
	std::deque<T> deque{};
	pthread_mutex_t lock;
	pthread_cond_t cond;
public:
	BlockingQueue() : lock(PTHREAD_MUTEX_INITIALIZER), cond(PTHREAD_COND_INITIALIZER) {}
	~BlockingQueue();
	T take();
	T &front();
	T &back();
	void put(const T&);
	void put(T&&);
	template< class... Args >
	void emplace_back(Args&&... args);
	void clear();
};

template<typename T>
BlockingQueue<T>::~BlockingQueue() {
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);
}

template<typename T>
T BlockingQueue<T>::take() {
	pthread_mutex_lock(&lock);
	while (deque.empty())
		pthread_cond_wait(&cond, &lock);
	T ret(std::move(deque.front()));
	deque.pop_front();
	pthread_mutex_unlock(&lock);
	return ret;
}

template<typename T>
void BlockingQueue<T>::put(const T &s) {
	pthread_mutex_lock(&lock);
	deque.push_back(s);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
}

template<typename T>
void BlockingQueue<T>::put(T &&s) {
	pthread_mutex_lock(&lock);
	deque.push_back(std::move(s));
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
}

template<typename T>
T &BlockingQueue<T>::front() {
	return deque.front();
}

template<typename T>
T &BlockingQueue<T>::back() {
	return deque.back();
}

template<typename T>
template<class... Args>
void BlockingQueue<T>::emplace_back(Args &&... args) {
	pthread_mutex_lock(&lock);
	deque.emplace_back(std::forward<Args>(args)...);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
}

template<typename T>
void BlockingQueue<T>::clear() {
	std::deque<T> t;
	deque.swap(t);
}
