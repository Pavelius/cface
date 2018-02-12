#include "collection.h"

#pragma once

void* rmreserve(void* ptr, unsigned size);
unsigned rmoptimal(unsigned size);
extern "C" void* memcpy(void* destination, const void* source, unsigned size);
extern "C" void* memset(void* destination, int value, unsigned size);

template<class T>
class arefc : public collection {
	T* data;
	unsigned count;
	unsigned count_maximum;
public:
	constexpr arefc() : data(0), count(0), count_maximum(0) {}
	~arefc() { if(data) delete data; }
	constexpr T& operator[](unsigned index) const { return data[index]; }
	operator bool() const { return count != 0; }
	void* add(const void* element) override {
		reserve(count + 1);
		auto p = data + (count++);
		if(element)
			memcpy(p, element, sizeof(T));
		else
			memset(p, 0, sizeof(T));
		return p;
	}
	T* add() { return (T*)add((const void*)0); }
	T* add(const T& element) { return (T*)add(&element); }
	T* begin() const { return data; }
	void clear() override { count = 0; }
	T* end() const { return data + count; }
	void* get(int index) const override { return data + index; }
	unsigned getcount() const override { return count; }
	unsigned getmaxcount() const override { return count_maximum; }
	unsigned getsize() const override { return sizeof(T); }
	int indexof(const void* element) const override {
		if((T*)element<data || (T*)element>data + count)
			return -1;
		return (T*)element - data;
	}
	void remove(int index, int elements_count = 1) override {
		if((unsigned)index >= count)
			return;
		count -= elements_count;
		if((unsigned)index >= count)
			return;
		memmove(data + index, data + index + elements_count, sizeof(data[0])*(count - index));
	}
	void reserve(unsigned count) {
		if(data) {
			if(count_maximum >= count)
				return;
		}
		count_maximum = rmoptimal(count);
		data = (T*)rmreserve(data, count_maximum*sizeof(T));
	}
};