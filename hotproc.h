#pragma once

struct hotproc {
	struct plugin {
		hotproc*		elements;
		plugin*			next;
		static plugin*	first;
		plugin(hotproc*	elements);
		static hotproc*	find(int key);
	};
	int					key; // Key to map
	void				(*proc)();
	operator bool() const { return key != 0; }
};
