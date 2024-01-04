#include "threadpool.h"

void threadpool::start(){
	uint32_t num_threads = std::thread::hardware_concurrency();
	//Why 20?
	//We aren't actually that concerned with performance.
	//Mostly this threadpool implementation is to get geolocation off the main thread which renders things.
	if(num_threads > 20){
		num_threads = 20;
	}
	for(uint32_t i = 0; i < num_threads; i++){
		threads.push_back(std::thread(&threadpool::threadloop, this));
	}
}

void threadpool::threadloop(){
	while(true){
		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			mutex_condition.wait(lock, [this] {
				return !jobs.empty() || should_terminate;
			});

			if(should_terminate){
				return;
			}

			job = jobs.front();
			jobs.pop();
		}
		job();
	}
}

void threadpool::add_job(const std::function<void()>& job){
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		jobs.push(job);
	}
	mutex_condition.notify_one();
}

bool threadpool::has_job(){
	bool has_job;
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		has_job = !jobs.empty();
	}
	return has_job;
}

void threadpool::stop(){
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		should_terminate = true;
	}
	mutex_condition.notify_all();

	for(std::thread& active_thread : threads){
		active_thread.join();
	}
	threads.clear();
}
