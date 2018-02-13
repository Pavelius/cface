#include "crt.h"
#include "amem.h"

void* rmreserve(void* ptr, unsigned size);
unsigned rmoptimal(unsigned size);

amem::amem(unsigned size)
	: data(0), count(0), size(size), count_maximum(0) {
}

amem::~amem() {
 	amem::clear();
}

void* amem::add() {
	reserve(count + 1);
	auto p = (char*)data + size*count;
	count++;
	return p;
}

void amem::clear() {
	data = rmreserve(data, 0);
	count = 0;
	count_maximum = 0;
}

void* amem::get(int index) const {
	return begin() + index*size;
}

void amem::reserve(unsigned count) {
	if(!size)
		return;
	if(data) {
		if(count_maximum >= count)
			return;
	}
	count_maximum = rmoptimal(count);
	data = rmreserve(data, count_maximum*size);
}

void amem::setup(unsigned size) {
	clear();
	this->size = size;
}

void* amem::insert(int index, const void* object) {
	reserve(count + 1);
	memmove(begin() + (index + 1)*size, begin() + index*size, (count - index)*size);
	void* p = (char*)data + index*size;
	if(object)
		memcpy(p, object, size);
	else
		memset(p, 0, size);
	count++;
	return p;
}

int amem::indexof(const void* element) const {
	if(size && element >= begin() && element < (char*)data + count*size)
		return ((char*)element - (char*)data) / size;
	return -1;
}

void amem::remove(int index, int elements_count) {
	if(((unsigned)index) >= count)
		return;
	if((unsigned)index < count - 1)
		memcpy((char*)data + index*size,
		(char*)data + (index + elements_count)*size,
			(count - (index + elements_count))*size);
	count -= elements_count;
}