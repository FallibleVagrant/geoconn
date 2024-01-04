#include "geolocation.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "debug.h"

#include <thread>

//TODO: delete this.
void* _get_in_addr(struct sockaddr* sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

void read_db(int AF, std::vector<struct geo_entry>& db, FILE* fd, std::atomic<long>& num_bytes_read){
	char line[256];
	unsigned int i = 0;
	while(fgets(line, sizeof(line), fd)){
		//dbgprint("Reading line from db; it is: %s.\n", line);
		if(i == 0){
			//Skip the header.
			i++;
			continue;
		}

		//Update the num_bytes_read.
		num_bytes_read = ftell(fd);

		//For each comma separated value.
		unsigned int j = 0;
		struct geo_entry entry;

		//Why not use strtok()?
		//The csv has empty values, and strtok() will skip over
		//two delimiters as though it were one.
		//We would like to count the number of columns, so we don't use it.
		char* p = line;
		char* q = p;
		while(true){
			for(p = q;; q++){
				if(*q == '\0'){
					p = NULL;
					break;
				}
				if(*q == ','){
					*q = '\0';
					q++;
					break;
				}
			}
			if(p == NULL){
				break;
			}
			//dbgprint("Reading entry from line; it is: %s.\n", p);
			switch(j){
				//Network range.
				case 0:
					{
					char* slashp = strchr(p, '/');
					if(slashp == NULL){
						dbgprint("Failed to read from GeoLite2 db: couldn't find slash in network range.\n");
						exit(1);
					}
					entry.cidr_bits = atoi(slashp + 1);
					if(entry.cidr_bits <= 0){
						dbgprint("Failed to read from GeoLite2 db: couldn't read cidr bits or else was 0 or negative.\n");
						exit(1);
					}
					*slashp = '\0';
					int r = inet_pton(AF, p, &(entry.range_addr));
					if(r != 1){
						dbgprint("Failed to read from GeoLite2 db: couldn't read addr: %s.\n", p);
						exit(1);
					}
					break;
					}
				//Latitude.
				case 7:
					{
					if(*p == '\0'){
						entry.lat = 0;
						break;
					}
					double temp = strtod(p, NULL);
					if(temp == 0){
						dbgprint("Failed to read from GeoLite2 db: couldn't read latitude or else was 0.\n");
						exit(1);
					}
					entry.lat = (float) temp;
					break;
					}
				//Longitude.
				case 8:
					{
					if(*p == '\0'){
						entry.lon = 0;
						break;
					}
					double temp = strtod(p, NULL);
					if(temp == 0){
						dbgprint("Failed to read from GeoLite2 db: couldn't read longitude or else was 0.\n");
						exit(1);
					}
					entry.lon = (float) temp;
					break;
					}
				//Accuracy range/radius.
				case 9:
					{
					if(*p == '\0'){
						entry.accuracy_radius = 0;
						break;
					}
					double temp = strtod(p, NULL);
					if(temp == 0){
						dbgprint("Failed to read from GeoLite2 db: couldn't read accuracy radius or else was 0.\n");
						exit(1);
					}
					entry.accuracy_radius = (float) temp;
					break;
					}
				default:
					;
					break;

			}

			j++;
		}
		i++;
		db.push_back(entry);
	}
}

void open_ipv4_db(FILE* fdv4, std::vector<struct geo_entry>& dbv4, std::atomic<bool>& is_ipv4_initialized, std::atomic<long>& num_bytes_read){
	read_db(AF_INET, dbv4, fdv4, num_bytes_read);

	fclose(fdv4);

	is_ipv4_initialized = true;
}

void open_ipv6_db(FILE* fdv6, std::vector<struct geo_entry>& dbv6, std::atomic<bool>& is_ipv6_initialized, std::atomic<long>& num_bytes_read){
	read_db(AF_INET6, dbv6, fdv6, num_bytes_read);

	fclose(fdv6);

	is_ipv6_initialized = true;
}

geolocation::geolocation(){
	//Open the files here so we can get the total_bytes before sending them to separate threads.

	dbgprint("Opening GeoLite2-City-Blocks-IPv4.csv...\n");
	FILE* fdv4 = fopen("GeoLite2-City-Blocks-IPv4.csv", "r");
	//FILE* fdv4 = fopen("debug4.csv", "r");
	if(fdv4 == 0){
		dbgprint("Could not open geolocation database.");
		exit(1);
	}

	fseek(fdv4, 0L, SEEK_END);
	total_bytes_fdv4 = ftell(fdv4);
	fseek(fdv4, 0L, SEEK_SET);

	dbgprint("Opening GeoLite2-City-Blocks-IPv6.csv...\n");
	FILE* fdv6 = fopen("GeoLite2-City-Blocks-IPv6.csv", "r");
	//FILE* fdv6 = fopen("debug6.csv", "r");
	if(fdv6 == 0){
		dbgprint("Could not open geolocation database.");
		exit(1);
	}

	fseek(fdv6, 0L, SEEK_END);
	total_bytes_fdv6 = ftell(fdv6);
	fseek(fdv6, 0L, SEEK_SET);

	//We *would* need to protect data with mutexes or something,
	//but we don't read these until they are fully loaded into memory anyways.
	//We use is_initialized() to indicate when they are done.
	//
	//Also we're only using one thread to read, which also means we don't have to protect the number of bytes read.
	std::thread tip4(open_ipv4_db, fdv4, std::ref(dbv4), std::ref(is_ipv4_initialized), std::ref(num_bytes_read_fdv4));
	std::thread tip6(open_ipv6_db, fdv6, std::ref(dbv6), std::ref(is_ipv6_initialized), std::ref(num_bytes_read_fdv6));

	tip4.detach();
	tip6.detach();
}

geolocation::~geolocation(){}

void dbgprint_addr(int AF, in6_addr addr){
	char buf[INET6_ADDRSTRLEN];
	sockaddr_storage* ss;
	sockaddr_in sin;
	sockaddr_in6 sin6;
	if(AF == AF_INET){
		sin.sin_family = AF_INET;
		memcpy(&(sin.sin_addr), &addr, 4);
		ss = (sockaddr_storage*) (&sin);
	}
	else{
		sin6.sin6_family = AF_INET6;
		memcpy(&(sin6.sin6_addr), &addr, 16);
		ss = (sockaddr_storage*) (&sin6);
	}
	inet_ntop(ss->ss_family, _get_in_addr((struct sockaddr*) ss), buf, INET6_ADDRSTRLEN);
	dbgprint("%s", buf);
}

bool is_ip_in_range(int AF, in6_addr addr, in6_addr range_addr, int cidr_bits){

	/*
dbgprint("Checking if ");
dbgprint_addr(AF, addr);
dbgprint(" is in range of ");
dbgprint_addr(AF, range_addr);
dbgprint("/%d.\n", cidr_bits);
*/

	if(AF == AF_INET){
//dbgprint("Working in IPv4.\n");
		//32 bits.
		uint32_t subnet_mask = 0;
		uint32_t addr_uint;
		uint32_t range_addr_uint;

		//Change from in6_addr to uint32_t so we can use ntohl().
		uint32_t addr_network_order;
		memcpy(&addr_network_order, &addr, 4);
		uint32_t range_addr_network_order;
		memcpy(&range_addr_network_order, &range_addr, 4);

//dbgprint("addr, in network order, is: %08X.\n", addr_network_order);
//dbgprint("range_addr, in network order, is: %08X.\n", range_addr_network_order);

		//addr_uint and range_addr_uint, in host order.
		addr_uint = ntohl(addr_network_order);
		range_addr_uint = ntohl(range_addr_network_order);

//dbgprint("addr, in host order, is: %08X.\n", addr_uint);
//dbgprint("range_addr, in host order, is: %08X.\n", range_addr_uint);

		//Create the subnet_mask.
		for(int i = 31; i >= 0; i--){
			if(cidr_bits == 0){
				break;
			}
			subnet_mask |= 1 << i;
			cidr_bits--;
		}

//dbgprint("The subnet_mask is: %08X.\n", subnet_mask);
//dbgprint("addr, in host order and &ed with subnet_mask is: %08X.\n", addr_uint & subnet_mask);
//dbgprint("range_addr, in host order and &ed with subnet_mask is: %08X.\n", range_addr_uint & subnet_mask);

		return (addr_uint & subnet_mask) == (range_addr_uint & subnet_mask);
	}
	else{
		//128 bits.
		uint32_t subnet_mask[4];
		uint32_t addr_uint[4];
		uint32_t range_addr_uint[4];

		memset(&subnet_mask, 0, 16);

		//Create addr_uint, which is addr except in host order.
		//Change from in6_addr to uint32_t so we can use ntohl().
		for(int i = 0; i < 4; i++){
			uint32_t network_order_chunk;
			memcpy(&network_order_chunk, (&addr) + (4*i), 4);

			addr_uint[i] = ntohl(network_order_chunk);
		}

		//Create range_addr_uint, which is range_addr in host order.
		//Change from in6_addr to uint32_t so we can use ntohl().
		for(int i = 0; i < 4; i++){
			uint32_t network_order_chunk;
			memcpy(&network_order_chunk, (&range_addr) + (4*i), 4);

			range_addr_uint[i] = ntohl(network_order_chunk);
		}

		//Create the subnet_mask.
		for(int i = 3; i >= 0; i--){
			if(cidr_bits >= 32){
				//Set to all ones.
				memset(&(subnet_mask[i]), 256, 4);
				cidr_bits -= 32;
				continue;
			}
			for(int bit_shift = 31; bit_shift >= 0; bit_shift--){
				if(cidr_bits == 0){
					break;
				}
				subnet_mask[i] |= 1 << bit_shift;
				cidr_bits--;
			}
		}

		//Compare.
		for(int i = 3; i >= 0; i--){
			if(subnet_mask[i] == 0){
				break;
			}
			if((addr_uint[i] & subnet_mask[i]) != (range_addr_uint[i] & subnet_mask[i])){
				return false;
			}
		}

		return true;
	}
}

int addrcmp(int AF, in6_addr addr1, in6_addr addr2){
	if(AF == AF_INET){
		return memcmp(&addr1, &addr2, sizeof(in_addr));
	}
	else{
		return memcmp(&addr1, &addr2, sizeof(in6_addr));
	}
}

//Thanks to:
//https://www.baeldung.com/linux/geolite2-ip-address-geolocation
//for the logic flow chart when my brain was very tired and perhaps a bit lazy.
int search(int AF, std::vector<struct geo_entry> db, in6_addr addr, float* lat, float* lon){
	dbgprint("Searching through the database...\n");
	unsigned int min = 0;
	unsigned int max = db.size() - 1;

	while(min <= max){
		unsigned int mid = (max + min) / 2;
		struct geo_entry entry;

		entry = db[mid];

		if(is_ip_in_range(AF, addr, entry.range_addr, entry.cidr_bits)){
dbgprint("IP resolved to these coords: %f, %f\n", entry.lat, entry.lon);
dbgprint("In range: ");
dbgprint_addr(AF, entry.range_addr);
dbgprint("/%d.\n", entry.cidr_bits);
			//Found it!
			*lat = entry.lat;
			*lon = entry.lon;
			return 1;
		}
		else{
//dbgprint("Hopped to: ");
//dbgprint_addr(AF, entry.range_addr);
//dbgprint("/%d", entry.cidr_bits);
		}

		//start_of_range_addr is ALWAYS the range_addr listed in the db.
		//in6_addr start_of_range_addr = get_start_of_range(AF, entry.range_addr, entry.cidr_bits);
		if(addrcmp(AF, addr, entry.range_addr) < 0){
			//Prevent segfault.
			if(mid == 0){
				return 0;
			}
			max = mid - 1;
		}
		else{
			min = mid + 1;
		}
	}

	return 0;
}

int geolocation::geolocate(sockaddr_storage ss, float* lat, float* lon){
	if(!is_initialized()){
		return -1;
	}

char buf[INET6_ADDRSTRLEN];
inet_ntop(ss.ss_family, _get_in_addr((struct sockaddr*) &ss), buf, INET6_ADDRSTRLEN);
dbgprint("Geolocating this address: %s.\n", buf);

	if(ss.ss_family == AF_INET){
		struct sockaddr_in* sa = (sockaddr_in*) &ss;
		in_addr addr;
		memcpy(&addr, &(sa->sin_addr), sizeof(in_addr));
		return search(AF_INET, dbv4, *((in6_addr*) &addr), lat, lon);
	}
	else{
		struct sockaddr_in6* sa = (sockaddr_in6*) &ss;
		in6_addr addr;
		memcpy(&addr, &(sa->sin6_addr), sizeof(in6_addr));
		return search(AF_INET6, dbv6, addr, lat, lon);
	}
}

bool geolocation::is_initialized(){
	return is_ipv4_initialized && is_ipv6_initialized;
}

void geolocation::get_database_file_lengths(long& tbfd4, long& tbfd6){
	tbfd4 = total_bytes_fdv4;
	tbfd6 = total_bytes_fdv6;
	return;
}

void geolocation::get_database_load_progress(long& nbrfd4, long& nbrfd6){
	nbrfd4 = num_bytes_read_fdv4;
	nbrfd6 = num_bytes_read_fdv6;
	return;
}
