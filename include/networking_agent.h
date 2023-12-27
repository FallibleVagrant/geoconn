#ifndef NETWORKING_RECEIVER_H
#define NETWORKING_RECEIVER_H

#include <vector>
#include <sys/socket.h>
#include <fcntl.h>
#include "ip_database.h"
#include "common.h"

struct client{
	unsigned long long client_id;
	int client_socket;
	bool has_requested_ips;
};

class networking_agent{
	public:
		networking_agent();
		~networking_agent();

		int init();
		int check_for_incoming_connections();
		int check_for_messages(ip_database& ip_db, std::vector<struct connection>& active_connections);
		int request_ips();
	private:
		int server_sock;
		unsigned long long seq_client_id;
		std::vector<struct client> clients;

		int init_socket();
		int add_client(int fd);
		void rem_client_by_index(int idx);
};

#endif
