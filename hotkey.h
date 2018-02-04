#pragma once

struct hotkey {
	int					key; // Key to map
	int					id; // Command
};
int						mapkey(hotkey* keys, int key);