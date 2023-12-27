//Thanks to beej's guide to networking for providing a quality introduction to sockets.
//(And also example code.)

#include "networking_agent.h"

#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

networking_agent::networking_agent(){}

#define DEFAULT_PORT "40343"
#define BACKLOG 10
#define BUFLEN 1024

int networking_agent::init_socket(){
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;

	//Use the loopback address because node is NULL and AI_PASSIVE isn't specified.
	//Cf. man page of getaddrinfo().
	//hints.ai_flags = AI_PASSIVE;
	
	const char* port = DEFAULT_PORT;
	
	struct addrinfo* servinfo;
	struct addrinfo* p;
	int r;
	if((r = getaddrinfo(NULL, port, &hints, &servinfo)) != 0){
		dbgprint("getaddrinfo: %s\n", gai_strerror(r));
		return -1;
	}

	//Loop through all the results and bind to the first we can.
	for(p = servinfo; p != NULL; p = p->ai_next){
		if((server_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			continue;
		}

		//Lose the pesky "address already in use" error message.
		int yes = 1;
		setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		//Set to non-blocking.
		if(fcntl(server_sock, F_SETFL, O_NONBLOCK) == -1){
			dbgprint("[NET_RECEIVER] Couldn't set socket to non-blocking for some reason.\n");
			return -1;
		}

		if(bind(server_sock, p->ai_addr, p->ai_addrlen) == -1){
			close(server_sock);
			continue;
		}

		break;
	}

	if(p == NULL){
		dbgprint("[NET_RECEIVER] Failed to bind a socket.\n");
		return -1;
	}

	dbgprint("Selected ai_family: %d\nSelected ai_socktype: %d\nSelected ai_protocol: %d\n", p->ai_family, p->ai_socktype, p->ai_protocol);

	freeaddrinfo(servinfo);

	//Now we listen().
	if(listen(server_sock, BACKLOG) == -1){
		dbgprint("[NET_RECEIVER] Failed to listen on its socket: %s.\n", strerror(errno));
		return -1;
	}

	return 0;
}

int networking_agent::init(){
	int r = init_socket();
	if(r == -1){
		return -1;
	}

	return 0;
}

networking_agent::~networking_agent(){
	for(struct client c : clients){
		close(c.client_socket);
	}

	close(server_sock);
}

//Get sockaddr, IPv6 or IPv4:
void* get_in_addr(struct sockaddr* sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

#include <limits.h>

int networking_agent::add_client(int client_socket){
	dbgprint("[NET_AGENT] Adding client connection.\n");

	//Set to non-blocking.
	if(fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1){
		dbgprint("[NET_AGENT] Couldn't set connecting socket to non-blocking for some reason.\n");
		return -1;
	}

	struct client c = {seq_client_id, client_socket, false};
	clients.push_back(c);

	seq_client_id++;
	if(seq_client_id >= ULLONG_MAX){
		dbgprint("[NET_AGENT] Somehow, more than 2^64 - 1 clients connected to this process in its lifetime. Exiting...\n");
		abort();
	}

	return 0;
}

void networking_agent::rem_client_by_index(int idx){
	struct client client = clients[idx];

	int client_socket = client.client_socket;
	close(client_socket);
	clients.erase(clients.begin() + idx);
}

int networking_agent::check_for_incoming_connections(){
	int connected_socket;
	struct sockaddr_storage rem_addr;
	socklen_t sin_size = sizeof(rem_addr);

	connected_socket = accept(server_sock, (struct sockaddr*) &rem_addr, &sin_size);
	if(connected_socket == -1){
		if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR){
			//Non-blocking and didn't find connection this time.
			//We'll look for more connections to accept() next round.
			return 0;
		}
		else{
			dbgprint("[NET_AGENT] Something went wrong with accepting a connection!\n");
			return -1;
		}
	}

	//Received a connection!
	char addr_str[INET6_ADDRSTRLEN];
	void* addr_bits = get_in_addr((struct sockaddr*) &rem_addr);
	dbgprint("[NET_AGENT] Connection received from %s.\n", inet_ntop(rem_addr.ss_family, addr_bits, addr_str, sizeof(addr_str)));

	int r = add_client(connected_socket);
	if(r == -1){
		return -1;
	}

	return 1;
}

int networking_agent::check_for_messages(ip_database& ip_db, std::vector<struct connection>& active_connections){
	int num_bytes;
	char buf[BUFLEN];

	for(unsigned int i = 0; i < clients.size(); i++){
		int client_socket = clients[i].client_socket;
		if((num_bytes = recv(client_socket, buf, BUFLEN - 1, 0)) == -1){
			//Non-blocking -- return early if the OS is giving problems; we'll get it next time.
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				return 0;
			}

			//The connection is erroring out!
			//Punt it to caller.
			return -2;
		}

		//Connection was closed.
		if(num_bytes == 0){
			return -2;
		}

		buf[BUFLEN - 1] = '\0';
		dbgprint("[NET_AGENT] Received a message!\n");

		char src_addr[INET6_ADDRSTRLEN];
		char dst_addr[INET6_ADDRSTRLEN];
		int ip_ver;

		for(char* bufp = strtok(buf, "\n"); bufp != NULL; bufp = strtok(NULL, "\n")){
			if(strncmp(bufp, "START", 5) == 0){
				active_connections.clear();
				continue;
			}

			int r = sscanf(bufp, "%d %64s %64s\n", &ip_ver, src_addr, dst_addr);
			if(r != 3){
				dbgprint("[NET_AGENT] Received an ill-formed IP. Skipping...\n");
				continue;
			}

			if(ip_ver == 4){
				ip_ver = AF_INET;
			}
			else if(ip_ver == 6){
				ip_ver = AF_INET6;
			}
			else{
				dbgprint("[NET_AGENT] IP version unrecognized. Skipping...\n");
				continue;
			}

			//Put src_addr in the IP database.
			sockaddr_storage src_ss;
			src_ss.ss_family = ip_ver;
			r = inet_pton(ip_ver, src_addr, get_in_addr((struct sockaddr*)&src_ss));
			if(r != 1){
				dbgprint("[NET_AGENT] Could not convert src_addr to bits.\n");
				continue;
			}

			ip_db.insert(src_ss);

			//Put dst_addr in the IP database.
			sockaddr_storage dst_ss;
			dst_ss.ss_family = ip_ver;
			r = inet_pton(ip_ver, dst_addr, get_in_addr((struct sockaddr*)&dst_ss));
			if(r != 1){
				dbgprint("[NET_AGENT] Could not convert dst_addr to bits.\n");
				continue;
			}

			ip_db.insert(dst_ss);

			//Since we just received this connection, it must be active.
			struct connection conn;
			conn.src = src_ss;
			conn.dst = dst_ss;
			active_connections.push_back(conn);
		}
	}

	return 0;
}

int networking_agent::request_ips(){
	return 0;
}
