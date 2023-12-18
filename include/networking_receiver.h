#ifndef NETWORKING_RECEIVER_H
#define NETWORKING_RECEIVER_H

#include <vector>
#include <sys/socket.h>
#include <fcntl.h>
#include "ip_database.h"
#include "common.h"

class networking_receiver{
	public:
		networking_receiver();
		~networking_receiver();

		int init();
		int check_for_messages(ip_database& ip_db, std::vector<struct connection>& active_connections);
	private:
		int client_sock;

		int init_socket();
};

#endif
