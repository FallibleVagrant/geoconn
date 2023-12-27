#ifndef GEOLOCATION_H
#define GEOLOCATION_H

#include <vector>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>

struct geo_entry{
	in6_addr range_addr;
	int cidr_bits;
	float lat;
	float lon;
	float accuracy_radius;
};

class geolocation{
	public:
		geolocation();
		~geolocation();

		int geolocate(sockaddr_storage ss, float* lat, float* lon);
	private:
		std::vector<struct geo_entry> dbv4;
		std::vector<struct geo_entry> dbv6;
};

#endif
