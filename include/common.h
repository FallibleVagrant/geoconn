#ifndef COMMON_H
#define COMMON_H

struct connection{
	sockaddr_storage src;
	sockaddr_storage dst;
};

#endif
