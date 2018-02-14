#include "adat.h"
#include "aref.h"
#include "collection.h"

#pragma once

void* rmreserve(void* ptr, unsigned size);
unsigned rmoptimal(unsigned size);

template<class T>
class arefc : public aref<T>, public collection {
	unsigned count_maximum;
public:
	arefc() : count_maximum(0) { data = 0; count = 0; }
	~arefc() { if(data) delete data; }
	void add(const T& e) { reserve(count + 1); aref<T>::add(e); }
	void* add() override { reserve(count + 1); return aref<T>::add(); }
	void clear() override { aref<T>::clear(); }
	void* get(int index) const override { return data + index; }
	unsigned getcount() const override { return count; }
	unsigned getmaxcount() const override { return count_maximum; }
	unsigned getsize() const override { return sizeof(T); }
	int indexof(const void* element) const override { return aref<T>::indexof((T*)element); }
	void remove(int index, int elements_count = 1) override { aref<T>::remove(index, elements_count); }
	void reserve(unsigned count) {
		if(data && count_maximum >= count)
			return;
		count_maximum = rmoptimal(count);
		data = (T*)rmreserve(data, count_maximum*sizeof(T));
	}
};

template<class T, unsigned count_maximum = 128>
struct adatc : adat<T, count_maximum>, collection {
	adatc() : { count = 0; }
	void* add() override { return adat<T, count_maximum>::add(); }
	void clear() override { adat<T, count_maximum>::clear(); }
	void* get(int index) const override { return data + index; }
	unsigned getcount() const override { return count; }
	unsigned getmaxcount() const override { return count_maximum; }
	unsigned getsize() const override { return sizeof(T); }
	int indexof(const void* element) const override { return adat<T, count_maximum>::indexof((T*)element); }
	void remove(int index, int elements_count = 1) override { adat<T, count_maximum>::remove(index, elements_count); }
};