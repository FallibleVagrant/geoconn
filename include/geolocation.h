#ifndef GEOLOCATION_H
#define GEOLOCATION_H

#include <vector>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <atomic>

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
		bool is_initialized();
		void get_database_file_lengths(long& total_bytes_fdv4, long& total_bytes_fdv6);
		void get_database_load_progress(long& num_bytes_read_fdv4, long& num_bytes_read_fdv6);
	private:
		std::atomic<bool> is_ipv4_initialized;
		std::atomic<bool> is_ipv6_initialized;

		std::atomic<long> total_bytes_fdv4;
		std::atomic<long> total_bytes_fdv6;
		std::atomic<long> num_bytes_read_fdv4;
		std::atomic<long> num_bytes_read_fdv6;

		std::vector<struct geo_entry> dbv4;
		std::vector<struct geo_entry> dbv6;
};

#endif
