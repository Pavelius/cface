#include "crt.h"
#include "draw.h"
#include "draw_tree.h"

using namespace draw::controls;

tree::tree(unsigned size) : amem(size), table(static_cast<collection&>(*this)),
sort_rows_by_name(false)
{
}

void tree::clear()
{
	amem::clear();
	current = 0;
}

int	tree::getparamindex(int value) const
{
	int count = getcount();
	for(int i = 0; i < count; i++)
	{
		auto p = (tree::element*)amem::get(i);
		if(p->param == value)
			return i;
	}
	return -1;
}

int tree::getlevel(int row) const
{
    auto t = static_cast<const element*>(amem::get(row));
	if(!t)
		return 0;
	return t->level;
}

int	tree::getparam(int row) const
{
	if(row == -1)
		row = current;
	auto t = static_cast<const element*>(amem::get(row));
	if(!t)
		return 0;
	return t->param;
}

int	tree::gettype(int row) const
{
	if(row == -1)
		row = current;
	auto t = static_cast<const element*>(amem::get(row));
	if(!t)
		return 0;
	return t->type;
}

bool tree::isgroup(int row) const
{
	auto t = static_cast<const element*>(amem::get(row));
	if(!t)
		return false;
	return (t->flags&TIGroup)!=0;
}

//int tree::execute(int id, bool run)
//{
//	int i;
//	switch(id)
//	{
//	case Clear:
//		if(run)
//		{
//			amem::clear();
//			current = 0;
//			maximum = 0;
//			correction();
//			ensurevisible();
//		}
//		break;
//	case Delete:
//		if(no_change_order)
//			return Disabled;
//		i = rows.getcount();
//		if(i == 0)
//			return Disabled;
//		if(run)
//		{
//			// Delete each child element
//			int elements_count = 1;
//			int rows_count = rows.getcount();
//			int current_level = getlevel(current);
//			for(int i = current + 1; i < rows_count; i++)
//			{
//				if(getlevel(i) <= current_level)
//					break;
//				elements_count++;
//			}
//			amem::remove(current, elements_count);
//			if(current >= i - 1)
//				current--;
//			correction();
//			ensurevisible();
//		}
//		break;
//	default:
//		return table::execute(id, run);
//	}
//	return Executed;
//}

//bool tree::get(const void* object, const menu& e, char* text) const
//{
//	if(amem::indexof(object)!=-1)
//		object = (void*)((element*)object)->param;
//	return table::get(object, e, text);
//}
//
//void tree::set(void* object, const menu& e, const char* value)
//{
//	if(amem::indexof(object)!=-1)
//		object = (void*)((element*)object)->param;
//	table::set(object, e, value);
//}

static int compare_by_name(const void* p1, const void* p2, void* param)
{
	auto pc = static_cast<tree*>(param);
	auto pe = pc->columns.find("name");
	if(!pe)
		pe = pc->columns.find("label");
	if(!pe)
		return 0;
	//char v1[260]; v1[0] = 0; pc->get(p1, *pe, v1);
	//char v2[260]; v2[0] = 0; pc->get(p2, *pe, v2);
	//return strcmp(v1, v2);
	return 0;
}

static int compare_by_name_group_up(const void* p1, const void* p2, void* param)
{
	auto pc = static_cast<tree*>(param);
	int g1 = (((draw::controls::tree::element*)p1)->flags&TIGroup) != 0 ? 1 : 0;
	int g2 = (((draw::controls::tree::element*)p2)->flags&TIGroup) != 0 ? 1 : 0;
	if(g1 == g2)
		return compare_by_name(p1, p2, param);
	return g2 - g1;
}

bool tree::haselement(int param) const
{
	auto level = this->level + 1;
	auto first = (const element*)((char*)amem::data);
	for(auto p = (const element*)((char*)amem::data + amem::size*index); p > first;
		p = (const element*)((char*)p - amem::size))
	{
		if(p->level != level)
			break;
		if(p->param == param)
			return true;
	}
	return false;
}

void tree::collapse(int i)
{
	// если выбранный элемент входит в сворачиваемю область
	// переместим его наверх
	int w = current;
	while(w != -1)
	{
		if(w == i)
			break;
		w = getparent(w);
	}
	int m = getlevel(i);
	int c = maximum;
	while((int)i < c)
	{
		if(i + 1 >= c || getlevel(i + 1) <= m)
			break;
		amem::remove(i + 1);
		if(current>(int)i)
			current--;
		c--;
	}
}

void tree::open(int max_level)
{
	for(int level = 1; level <= max_level; level++)
	{
		bool need_test = true;
		while(need_test)
		{
			need_test = false;
			int c = rows.getcount();
			for(int i = 0; i < c; i++)
			{
				if(level != getlevel(i))
					continue;
				if(i < c - 1)
				{
					if(getlevel(i + 1) > level)
						continue;
				}
				if(isgroup(i))
				{
					unsigned i1 = rows.getcount();
					expand(i, level);
					if(i1 < rows.getcount())
					{
						need_test = true;
						break;
					}
				}
			}
		}
	}
}

void tree::expand(int index, int level)
{
	this->index = index;
	this->level = level;
	expanding();
	if(sort_rows_by_name)
	{
		if(level==0)
			amem::sort(0, amem::count-1, group_sort_up ? compare_by_name_group_up : compare_by_name, this);
		else
			amem::sort(index + 1, this->index, group_sort_up ? compare_by_name_group_up : compare_by_name, this);
	}
}

void tree::addrow(tree::element& e)
{
	e.level = level + 1;
	if(level==0)
		amem::add(&e);
	else
		amem::insert(++index, &e);
}

void tree::addrow(unsigned param, unsigned char flags, unsigned char type, unsigned char image)
{
	element e;
	e.param = param;
	e.image = image;
	e.flags = flags;
	e.type = type;
	addrow(e);
}

void tree::addrow(void* object)
{
	if(size!=sizeof(element))
		return;
	element e = {0};
	e.param = (int)object;
	rows.add(&e);
}