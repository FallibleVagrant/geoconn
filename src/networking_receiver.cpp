//Thanks to beej's guide to networking for providing a quality introduction to sockets.
//(And also example code.)

#include "networking_receiver.h"

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

networking_receiver::networking_receiver(){}

networking_receiver::~networking_receiver(){
	close(client_sock);
}

void* get_in_addr(struct sockaddr* sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

#define DEFAULT_PORT "40344"
#define BACKLOG 10
#define BUFLEN 1024

int networking_receiver::init_socket(){
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
		dbgprint("[NET_RECEIVER] getaddrinfo() returned err: %s\n", gai_strerror(r));
		return -1;
	}

	//Loop through all the results and connect to the first we can.
	for(p = servinfo; p != NULL; p = p->ai_next){
		if((client_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			dbgprint("[DEBUG] Couldn't get a socket for some reason.\n");
			continue;
		}

		/*
		//Set to non-blocking.
		if(fcntl(client_sock, F_SETFL, O_NONBLOCK) == -1){
			dbgprint("[NET_RECEIVER] couldn't set socket to non-blocking for some reason.\n");
			return -1;
		}
		*/

		if(connect(client_sock, p->ai_addr, p->ai_addrlen) == -1){
			dbgprint("[DEBUG] Couldn't connect() for some reason.\n");
			close(client_sock);
			continue;
		}

		break;
	}

	if(p == NULL){
		dbgprint("[NET_RECEIVER] Failed to connect.\n");
		return -1;
	}

	//Set to non-blocking.
	if(fcntl(client_sock, F_SETFL, O_NONBLOCK) == -1){
		dbgprint("[NET_RECEIVER] couldn't set socket to non-blocking for some reason.\n");
		return -1;
	}

	dbgprint("Selected ai_family: %d\nSelected ai_socktype: %d\nSelected ai_protocol: %d\n", p->ai_family, p->ai_socktype, p->ai_protocol);
	char s[INET6_ADDRSTRLEN];
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof(s));
	dbgprint("Connected to: %s", s);

	freeaddrinfo(servinfo);

	//Send a notice that we want IPs.
	int num_bytes;
	const char* buf = "REQ IP";
	if((num_bytes = send(client_sock, buf, strlen(buf), 0)) == -1){
		return -1;
	}

	return 0;
}

int networking_receiver::init(){
	int r = init_socket();
	if(r == -1){
		return -1;
	}

	return 0;
}

int networking_receiver::check_for_messages(ip_database& ip_db, std::vector<struct connection>& active_connections){
	int num_bytes;
	char buf[BUFLEN];

	if((num_bytes = recv(client_sock, buf, BUFLEN - 1, 0)) == -1){
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
	dbgprint("[NET_RECEIVER] Received a message!\n");

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
			dbgprint("[NET_RECEIVER] Received an ill-formed IP. Skipping...\n");
			continue;
		}

		if(ip_ver == 4){
			ip_ver = AF_INET;
		}
		else if(ip_ver == 6){
			ip_ver = AF_INET6;
		}
		else{
			dbgprint("[NET_RECEIVER] IP version unrecognized. Skipping...\n");
			continue;
		}

		//Put src_addr in the IP database.
		sockaddr_storage src_ss;
		src_ss.ss_family = ip_ver;
		r = inet_pton(ip_ver, src_addr, get_in_addr((struct sockaddr*)&src_ss));
		if(r != 1){
			dbgprint("[NET_RECEIVER] Could not convert src_addr to bits.\n");
			continue;
		}

		ip_db.insert(src_ss);

		//Put dst_addr in the IP database.
		sockaddr_storage dst_ss;
		dst_ss.ss_family = ip_ver;
		r = inet_pton(ip_ver, dst_addr, get_in_addr((struct sockaddr*)&dst_ss));
		if(r != 1){
			dbgprint("[NET_RECEIVER] Could not convert dst_addr to bits.\n");
			continue;
		}
		ip_db.insert(dst_ss);

		//Since we just received this connection, it must be active.
		struct connection conn;
		conn.src = src_ss;
		conn.dst = dst_ss;
		active_connections.push_back(conn);
	}

	return 0;
}
