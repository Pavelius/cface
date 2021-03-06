#include "crt.h"
#include "draw.h"
#include "draw_tree.h"

using namespace draw::controls;

tree::tree(unsigned size) : amem(size), table(static_cast<collection&>(*this)),
sort_rows_by_name(false), element_param_have_row(false) {
}

void tree::clear() {
	amem::clear();
	current = 0;
}

int	tree::getparamindex(int value) const {
	int count = getcount();
	for(int i = 0; i < count; i++) {
		auto p = (tree::element*)amem::get(i);
		if(p->param == value)
			return i;
	}
	return -1;
}

int tree::getlevel(int row) const {
	auto t = static_cast<const element*>(amem::get(row));
	if(!t)
		return 0;
	return t->level;
}

int	tree::getparam(int row) const {
	if(row == -1)
		row = current;
	auto t = static_cast<const element*>(amem::get(row));
	if(!t)
		return 0;
	return t->param;
}

int	tree::findbyparam(int param) const {
	unsigned i2 = rows.getcount();
	for(unsigned i = 0; i < i2; i++) {
		auto p = (element*)rows.get(i);
		if(p->param == param)
			return i;
	}
	return -1;
}

int	tree::gettype(int row) const {
	if(row == -1)
		row = current;
	auto t = static_cast<const element*>(amem::get(row));
	if(!t)
		return 0;
	return t->type;
}

bool tree::isgroup(int row) const {
	auto t = static_cast<const element*>(amem::get(row));
	if(!t)
		return false;
	return (t->flags&TIGroup) != 0;
}

void* tree::getrow(int index) {
	if(element_param_have_row)
		return (void*)((element*)rows.get(index))->param;
	return table::getrow(index);
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

static int compare_by_name(const void* p1, const void* p2, void* param) {
	auto pc = static_cast<tree*>(param);
	//auto pe = pc->columns.find("name");
	//if(!pe)
	//	pe = pc->columns.find("label");
	//if(!pe)
	//	return 0;
	//char v1[260]; v1[0] = 0; pc->get(p1, *pe, v1);
	//char v2[260]; v2[0] = 0; pc->get(p2, *pe, v2);
	//return strcmp(v1, v2);
	return 0;
}

static int compare_by_name_group_up(const void* p1, const void* p2, void* param) {
	auto pc = static_cast<tree*>(param);
	int g1 = (((draw::controls::tree::element*)p1)->flags&TIGroup) != 0 ? 1 : 0;
	int g2 = (((draw::controls::tree::element*)p2)->flags&TIGroup) != 0 ? 1 : 0;
	if(g1 == g2)
		return compare_by_name(p1, p2, param);
	return g2 - g1;
}

bool tree::haselement(int param) const {
	auto level = this->level + 1;
	auto first = (const element*)amem::begin();
	for(auto p = (const element*)amem::end(); p > first;
		p = (const element*)((char*)p - getsize())) {
		if(p->level != level)
			break;
		if(p->param == param)
			return true;
	}
	return false;
}

void tree::collapse(int i) {
	// ���� ��������� ������� ������ � ������������ �������
	// ���������� ��� ������
	int w = current;
	while(w != -1) {
		if(w == i)
			break;
		w = getparent(w);
	}
	int m = getlevel(i);
	int c = maximum;
	while((int)i < c) {
		if(i + 1 >= c || getlevel(i + 1) <= m)
			break;
		amem::remove(i + 1);
		if(current > (int)i)
			current--;
		c--;
	}
}

void tree::open(int max_level) {
	for(int level = 1; level <= max_level; level++) {
		bool need_test = true;
		while(need_test) {
			need_test = false;
			int c = rows.getcount();
			for(int i = 0; i < c; i++) {
				if(level != getlevel(i))
					continue;
				if(i < c - 1) {
					if(getlevel(i + 1) > level)
						continue;
				}
				if(isgroup(i)) {
					unsigned i1 = rows.getcount();
					expand(i, level);
					if(i1 < rows.getcount()) {
						need_test = true;
						break;
					}
				}
			}
		}
	}
}

void tree::expand(int index, int level) {
	this->index = index;
	this->level = level;
	expanding();
	if(this->index != index) {
		auto p = (element*)rows.get(index);
		p->flags |= TIGroup;
	}
	// Remove unused rows
	unsigned i1 = this->index + 1;
	unsigned i2 = i1;
	while(i2<rows.getcount()) {
		auto p2 = (element*)rows.get(i2);
		if(p2->level >= (level+1))
			i2++;
		else
			break;
	}
	if(i1 != i2)
		rows.remove(i1, i2 - i1);
	// Finally sort all rows
	if(sort_rows_by_name) {
		if(level == 0)
			amem::sort(0, getcount() - 1, group_sort_up ? compare_by_name_group_up : compare_by_name, this);
		else
			amem::sort(index + 1, this->index, group_sort_up ? compare_by_name_group_up : compare_by_name, this);
	}
}

void tree::addrow(tree::element& e) {
	e.level = level + 1;
	if((unsigned)(index + 1) < rows.getcount()) {
		auto p = (element*)rows.get(index + 1);
		if(p->level >= e.level) {
			memcpy(p, &e, getsize());
			index++;
			return;
		}
	}
	if(level == 0)
		collection::add(&e);
	else
		amem::insert(++index, &e);
}

void tree::addrow(unsigned param, unsigned char flags, unsigned char type, unsigned char image) {
	element e;
	e.param = param;
	e.image = image;
	e.flags = flags;
	e.type = type;
	addrow(e);
}

void tree::addrow(void* object) {
	if(getsize() != sizeof(element))
		return;
	element e = {0};
	e.param = (int)object;
	rows.add(&e);
}

unsigned tree::sortas(bool run) {
	if(no_change_order)
		return Disabled;
	if(rows.getcount() < 2)
		return Disabled;
	if(run) {
		auto parent = getparent(current);
		auto i1 = parent + 1;
		auto i2 = getlastchild(parent);
		table::sort(columns[current_column].id, 1, i1, i2);
	}
	return 0;
}

unsigned tree::sortds(bool run) {
	if(no_change_order)
		return Disabled;
	if(rows.getcount() < 2)
		return Disabled;
	if(run) {
		auto parent = getparent(current);
		auto i1 = parent + 1;
		auto i2 = getlastchild(parent);
		table::sort(columns[current_column].id, -1, i1, i2);
	}
	return 0;
}

int	tree::getimageindex(int index) const {
	return ((element*)rows.get(index))->image;
}