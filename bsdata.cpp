#include "crt.h"
#include "bsdata.h"

bsdata* bsdata::first;

bsdata::bsdata(const char* id, void* data, unsigned size, unsigned maximum_count, const bsreq* fields) :
	id(id), fields(fields), data(data), size(size), maximum_count(maximum_count), current_count(maximum_count), count(current_count) {
	seqlink(this);
}

bsdata::bsdata(const char* id, void* data, unsigned& count, unsigned size, unsigned maximum_count, const bsreq* fields) :
	id(id), fields(fields), data(data), size(size), maximum_count(maximum_count), current_count(0), count(count) {
	seqlink(this);
}

void* bsdata::add(const void* object) {
	if(getcount() >= getmaxcount())
		return 0;
	auto p = (void*)get(count++);
	if(object)
		memcpy(p, object, size);
	return p;
}

bsdata* bsdata::find(const char* id) {
	if(!id || !id[0])
		return 0;
	for(auto p = first; p; p = p->next) {
		if(strcmp(p->id, id) == 0)
			return p;
	}
	return 0;
}

bsdata* bsdata::find(const bsreq* type) {
	if(!type)
		return 0;
	for(auto p = first; p; p = p->next) {
		if(p->fields == type)
			return p;
	}
	return 0;
}

int bsdata::indexof(const void* object) const {
	if(object >= data && object <= (char*)data + maximum_count*size)
		return ((char*)object - (char*)data) / size;
	return -1;
}

void bsdata::remove(int index, int elements_count) {
	if(((unsigned)index) >= getcount())
		return;
	if((unsigned)index < getcount() - 1)
		memcpy((char*)data + index*size,
		(char*)data + (index + elements_count)*size,
			(getcount() - (index + elements_count))*size);
	count -= elements_count;
}

void bsdata::swap(int i1, int i2) {
	auto p1 = (char*)data + i1*size;
	auto p2 = (char*)data + i2*size;
	auto pz = p1 + size;
	while(p1 < pz)
		iswap(*p1++, *p2++);
}

bsdata* bsdata::findbyptr(const void* object) {
	if(!object)
		return 0;
	for(auto p = first; p; p = p->next)
		if(p->indexof(object) != -1)
			return p;
	return 0;
}

void* bsdata::find(const bsreq* id, const char* value) {
	if(!id || id->type!=text_type)
		return 0;
	auto ps = (char*)id->ptr(data);
	auto pe = ps + size*getcount();
	for(; ps<pe; ps += size) {
		auto ps_value = (const char*)id->get(ps);
		if(!ps_value)
			continue;
		if(strcmp(ps_value, value) == 0)
			return get(indexof(ps));
	}
	return 0;
}