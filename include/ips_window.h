#ifndef IPS_WINDOW_H
#define IPS_WINDOW_H

#include "ip_database.h"

class ips_window{
	public:
		ips_window();
		~ips_window();

		void render(ip_database& ip_db);
	private:
		unsigned int selected_ip;
};

#endif
