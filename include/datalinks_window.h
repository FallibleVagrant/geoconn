#ifndef DATALINKS_WINDOW_H
#define DATALINKS_WINDOW_H

class datalinks_window{
	public:
		datalinks_window();
		~datalinks_window();

		void render(bool& trying_to_connect, unsigned int num_data_sources);
	private:
		int template_current_idx;
};

#endif
