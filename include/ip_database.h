#ifndef IP_DATABASE_H
#define IP_DATABASE_H

#include <vector>
#include <sys/socket.h>

struct ip_entry{
	sockaddr_storage ss;
};

class ip_database{
	public:
		ip_database();
		~ip_database();

		void insert(sockaddr_storage ss);
		bool contains(sockaddr_storage ss);
		std::vector<struct ip_entry>& get_db();
	private:
		//This probably shouldn't be an array.
		std::vector<struct ip_entry> db;
};

#endif
