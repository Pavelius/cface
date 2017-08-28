#include "crt.h"
#include "pathfind.h"

unsigned char			max_path_width;
unsigned char			max_path_height;
static unsigned short	path_route[256*256];
static unsigned short	path_stack[256*256];
static unsigned short	path_push;
static unsigned short	path_pop;
static unsigned short	path_goal;
static unsigned short	path_start;
static unsigned short	path_cost;
static int				path_size;
static pathnode			objects[1024*8];

pathnode* create_node()
{
	for(auto& e : objects)
	{
		if(!e.index)
		{
			e.next = 0;
			return &e;
		}
	}
	objects[0].index = 0;
	objects[0].next = 0;
	return objects;
}

int get_node_count()
{
	int result = 0;
	for(auto& e : objects)
	{
		if(e.index)
			result++;
	}
	return result;
}

unsigned short get_path_cost(unsigned short  index)
{
	return path_route[index];
}

static void snode(unsigned short index)
{
	if(index==Blocked)
		return;
	auto a = path_route[index];
	if(a==Blocked)
		return;
	if(path_size>1 && is_block_movement(index, path_size))
	{
		path_route[index] = Blocked;
		return;
	}
	int cost = path_cost + get_move_cost(index);
	if(a && cost>=a)
		return;
	path_stack[path_push++] = index;
	path_route[index] = cost;
}

static void gnext(int index, unsigned short& level, int& pos)
{
	if(index==-1)
		return;
	auto nlevel = path_route[index];
	if(nlevel==Blocked)
		return;
	if(!nlevel)
		return;
	if(nlevel <= level)
	{
		level = nlevel;
		pos = index;
	}
}

move_directions vectorized(move_directions d, move_directions d1)
{
	switch(d)
	{
	case Up:
		return d1;
	case Left:
		switch(d1)
		{
		case Left: return Down;
		case Up: return Left;
		case Right: return Up;
		case Down: return Right;
		}
		break;
	case Right:
		switch(d1)
		{
		case Left: return Up;
		case Up: return Right;
		case Right: return Down;
		case Down: return Left;
		}
		break;
	case Down:
		switch(d1)
		{
		case Left: return Right;
		case Up: return Down;
		case Right: return Left;
		case Down: return Up;
		}
		break;
	}
	return Center;
}

move_directions rotateto(move_directions d, move_directions d1)
{
	switch(d1)
	{
	case Left:
		switch(d)
		{
		case Left: return Down;
		case Up: return Left;
		case Right: return Up;
		case Down: return Right;
		}
		break;
	case Right:
		switch(d)
		{
		case Left: return Up;
		case Up: return Right;
		case Right: return Down;
		case Down: return Left;
		}
		break;
	}
	return Center;
}

unsigned short moveto(int index, move_directions d)
{
	switch(d)
	{
	case Left:
		if((index&0xFF)==0)
			return Blocked;
		return index-1;
	case LeftUp:
		if((index&0xFF)==0)
			return Blocked;
		if((index>>8)==0)
			return Blocked;
		return index-1-256;
	case Up:
		if((index>>8)==0)
			return Blocked;
		return index-256;
	case RightUp:
		if((index&0xFF)>=max_path_width-1)
			return Blocked;
		if((index>>8)==0)
			return Blocked;
		return index-256+1;
	case Right:
		if((index&0xFF)>=max_path_width-1)
			return Blocked;
		return index+1;
	case RightDown:
		if((index&0xFF)>=max_path_width-1)
			return Blocked;
		if((index>>8)>=max_path_height-1)
			return Blocked;
		return index+1;
	case Down:
		if((index>>8)>=max_path_height-1)
			return Blocked;
		return index+256;
	case LeftDown:
		if((index&0xFF)==0)
			return Blocked;
		if((index>>8)>=max_path_height-1)
			return Blocked;
		return index-1+256;
	default:
		return Blocked;
	}
}

void block_impassable_route()
{
	int iy2 = max_path_height*256;
	for(int iy = 0; iy<iy2; iy += 256)
	{
		int ix = iy + max_path_width;
		for(int i = iy; i<ix; i++)
			path_route[i] = is_block_movement(i) ? Blocked : 0;
	}
}

bool is_path_passable(unsigned short i0, unsigned short i1, int size)
{
	int x0 = i0%256; int y0 = i0/256;
	int x1 = i1%256; int y1 = i1/256;
	int dx = iabs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = -iabs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = dx+dy, e2;
	for(;;)
	{
		e2 = 2*err;
		if(e2 >= dy)
		{
			if(x0 == x1)
				break;
			err += dy;
			if(e2 <= dx)
			{
				if(is_block_movement(get_path_index(x0, y0+sy), size))
					return false;
			}
			x0 += sx;
			if(is_block_movement(get_path_index(x0, y0), size))
				return false;
		}
		if(e2 <= dx)
		{
			if(y0 == y1)
				break;
			err += dx;
			if(2*err >= dy)
			{
				if(is_block_movement(get_path_index(x0+sx, y0), size))
					return false;
			}
			y0 += sy;
			if(is_block_movement(get_path_index(x0, y0), size))
				return false;
		}
	}
	return true;
}

pathnode* remove_all_nodes(pathnode* node)
{
	while(node)
	{
		node->index = 0;
		node = node->next;
	}
	return 0;
}

pathnode* remove_one_node(pathnode* p)
{
	auto p1 = p->next;
	p->index = 0;
	return p1;
}

int get_path_distance(unsigned short i0, unsigned short i1)
{
	int x0 = i0%256; int y0 = i0/256;
	int x1 = i1%256; int y1 = i1/256;
	return imax(iabs(x0-x1), iabs(y0-y1));
}

pathnode* create_path_nodes(short unsigned start, int size)
{
	auto s = path_stack + path_push - 2;
	auto p = create_node();
	auto result = p;
	auto dest = start;
	while(s>=path_stack)
	{
		if(is_path_passable(start, *s, size))
			dest = *s;
		else
		{
			p->index = dest;
			p->next = create_node();
			p = p->next;
			start = dest;
			dest = *s;
		}
		s--;
	}
	p->index = dest;
	return result;
}

// First, make wave and see what cell on map is passable
void create_wave(short unsigned start, int size)
{
	block_impassable_route();
	if(start==-1)
		return;
	path_push = 0;
	path_pop = 0;
	path_stack[path_push++] = start;
	path_route[start] = 1;
	path_size = size;
	while(path_push != path_pop)
	{
		auto pos = path_stack[path_pop++];
		auto cost = path_route[pos];
		if(cost>=0xFF00)
			break;
		path_cost = cost;
		snode(moveto(pos, Left));
		snode(moveto(pos, Right));
		snode(moveto(pos, Up));
		snode(moveto(pos, Down));
		//path_cost += 1;
		//snode(moveto(pos, LeftUp));
		//snode(moveto(pos, LeftDown));
		//snode(moveto(pos, RightUp));
		//snode(moveto(pos, RightDown));
	}
	path_pop = 0;
	path_push = 0;
	path_goal = -1;
	path_start = start;
}

// Calculate path step by step to any cell on map analizing create_wave result.
// Go form goal to start and get lowest weight.
// When function return 'path_stack' has step by step path and 'path_push' is top of this path.
bool route_path(short unsigned start, short unsigned goal)
{
	path_push = 0;
	path_goal = -1;
	int pos = goal;
	unsigned short level = Blocked;
	path_stack[path_push++] = goal;
	while(pos != start)
	{
		auto n = pos;
		gnext(moveto(pos, Left), level, n);
		gnext(moveto(pos, Right), level, n);
		gnext(moveto(pos, Up), level, n);
		gnext(moveto(pos, Down), level, n);
		//gnext(moveto(pos, LeftDown), level, n);
		//gnext(moveto(pos, LeftUp), level, n);
		//gnext(moveto(pos, RightDown), level, n);
		//gnext(moveto(pos, RightUp), level, n);
		if(pos==n)
			return false;
		pos = n;
		path_stack[path_push++] = n;
		level = path_route[pos];
		if(path_push>65000)
			return false;
	}
	path_goal = goal;
	return true;
}

static bool get_free_space_x(int& index, int radius, int size)
{
	int px = index&0xFF;
	int py = index>>8;
	int minx = imax(px - radius, 0);
	int maxx = imin(px + radius + 1, (int)max_path_width);
	for(int scanx = minx; scanx < maxx; scanx++)
	{
		if(py>=radius)
		{
			int i = get_path_index(scanx, py - radius);
			if(!is_block_movement(i))
			{
				index = i;
				return true;
			}
		}
		if(py+radius<max_path_height)
		{
			int i = get_path_index(scanx, py + radius);
			if(!is_block_movement(i))
			{
				index = i;
				return true;
			}
		}
	}
	return false;
}

static bool get_free_space_y(int& index, int radius, int size)
{
	int px = index&0xFF;
	int py = index>>8;
	int miny = imax(py - radius, 0);
	int maxy = imin(py + radius + 1, (int)max_path_height);
	for(int scany = miny; scany < maxy; scany++)
	{
		if(px>=radius)
		{
			int i = get_path_index(px - radius, scany*256);
			if(!is_block_movement(i, size))
			{
				index = i;
				return true;
			}
		}
		if(px+radius<max_path_width)
		{
			int i = get_path_index(px + radius, scany*256);
			if(!is_block_movement(i, size))
			{
				index = i;
				return true;
			}
		}
	}
	return false;
}

int get_free_index(int index, int radius, int size)
{
	int maxr = max_path_width/2;
	if(maxr > max_path_height)
		maxr = max_path_height;
	for(; radius < maxr; radius++)
	{
		if(rand()&1)
		{
			if(get_free_space_x(index, radius, size))
				break;
			if(get_free_space_y(index, radius, size))
				break;
		}
		else
		{
			if(get_free_space_y(index, radius, size))
				break;
			if(get_free_space_x(index, radius, size))
				break;
		}
	}
	return index;
}