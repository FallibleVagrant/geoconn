#include "ip_database.h"

#include <stdlib.h>

ip_database::ip_database(){}

ip_database::~ip_database(){
	//TODO: free this structure!
}

#include <netdb.h>
#include <string.h>

#include "debug.h"
#include <arpa/inet.h>

void ip_database::insert(sockaddr_storage ss){
	if(contains(ss)){
		return;
	}
	//Check for 0.0.0.0 and 127.0.0.1.
	if(ss.ss_family == AF_INET){
		if(((struct sockaddr_in*) &ss)->sin_addr.s_addr == htonl(INADDR_LOOPBACK)
			|| ((struct sockaddr_in*) &ss)->sin_addr.s_addr == htonl(INADDR_ANY)
			|| ((struct sockaddr_in*) &ss)->sin_addr.s_addr == htonl(INADDR_BROADCAST)
			|| ((struct sockaddr_in*) &ss)->sin_addr.s_addr == htonl(INADDR_NONE)){
			return;
		}
	}
	//Check for :: and ::1.
	else if(ss.ss_family == AF_INET6){
		if(memcmp(&(((struct sockaddr_in6*) &ss)->sin6_addr), &in6addr_loopback, sizeof(struct in6_addr)) == 0
			|| memcmp(&(((struct sockaddr_in6*) &ss)->sin6_addr), &in6addr_any, sizeof(struct in6_addr)) == 0){
			return;
		}
	}

	struct ip_entry ipe;
	ipe.ss = ss;
	ipe.lat = 0;
	ipe.lon = 0;

	int r = geo.geolocate(ss, &(ipe.lat), &(ipe.lon));
	if(r == 0){
		dbgprint("Could not resolve addr.\n");
	}

	db.push_back(ipe);
}

#include <netdb.h>
#include <string.h>

bool ip_database::contains(sockaddr_storage ss){
	for(struct ip_entry ipe : db){
		sockaddr_storage dbss = ipe.ss;
		if(dbss.ss_family != ss.ss_family){
			continue;
		}

		if(ss.ss_family == AF_INET){
			struct sockaddr_in* sap = (sockaddr_in*) &ss;
			struct sockaddr_in* dbsap = (sockaddr_in*) &dbss;
			if(sap->sin_addr.s_addr == dbsap->sin_addr.s_addr){
				return true;
			}
		}
		else{
			struct sockaddr_in6* sap = (sockaddr_in6*) &ss;
			struct sockaddr_in6* dbsap = (sockaddr_in6*) &dbss;
			if(memcmp(&(sap->sin6_addr), &(dbsap->sin6_addr), sizeof(in6_addr)) == 0){
				return true;
			}
		}
	}
	
	return false;
}

std::vector<struct ip_entry> ip_database::get_db(){
	return db;
}
