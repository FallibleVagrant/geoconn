#ifndef IP_DATABASE_H
#define IP_DATABASE_H

#include <vector>
#include <queue>
#include <sys/socket.h>
#include <stdio.h>
#include <mutex>

#include "geolocation.h"
#include "threadpool.h"

struct ip_entry{
	sockaddr_storage ss;
	float lat;
	float lon;
};

class ip_database{
	public:
		ip_database(geolocation& geo);
		~ip_database();

		void insert(sockaddr_storage ss);
		bool contains(sockaddr_storage ss);
		std::vector<struct ip_entry> get_db();

		//TEMP
		std::queue<sockaddr_storage> get_processing_queue();
	private:
		//This probably shouldn't be an array.
		std::mutex db_mutex;
		std::vector<struct ip_entry> db;

		std::queue<sockaddr_storage> processing_queue;

		geolocation& geo;
		threadpool tpool;
};

#endif
