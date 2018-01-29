#pragma once

enum black_states {
	Blocked = 0xFFFF
};
enum move_directions {
	Center,
	Left, Up, Right, Down,
	LeftUp, RightUp, RightDown, LeftDown
};
struct pathnode {
	short unsigned		index;
	pathnode*			next;
};

// Create one movement node.
pathnode*				create_node();
// Create movement nodes using result of route_path() function.
pathnode*				create_path_nodes(short unsigned start, int size);
// Every pathfind operation start with this function. It'is very slow.
// Function run wave to game board and mark each cell by integer value.
// This integer value is movement cost.
void					create_wave(short unsigned start, int size);
// When you find free cell in 'radius' that can fit object.
int						get_free_index(int index, int radius, int size);
// Return current path cost on board.
// Can be used when alnalize game board, finding best movement possibilities.
extern unsigned short	get_move_cost(unsigned short index);
// Return count of currently used nodes. Used for memory leak analize.
int						get_node_count();
unsigned short			get_path_cost(unsigned short index);
inline int				get_path_index(int x, int y) { return y * 256 + x; }
extern bool				is_block_movement(unsigned short index);
extern bool				is_block_movement(unsigned short index, int size);
extern unsigned char	max_path_width; // current map width
extern unsigned char	max_path_height; // current map height
unsigned short			moveto(int index, move_directions direction);
pathnode*				remove_all_nodes(pathnode* p);
pathnode*				remove_one_node(pathnode* p);
bool					route_path(short unsigned start, short unsigned goal);
move_directions			rotateto(move_directions d, move_directions d1);
move_directions			vectorized(move_directions d, move_directions d1);