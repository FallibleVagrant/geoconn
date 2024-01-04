#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <queue>

class threadpool{
	public:
		void start();
		void add_job(const std::function<void()>& job);
		bool has_job();
		void stop();
	private:
		void threadloop();

		bool should_terminate = false;
		std::mutex queue_mutex;
		std::condition_variable mutex_condition;
		std::vector<std::thread> threads;
		std::queue<std::function<void()>> jobs;
};



#endif
