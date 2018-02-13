#include "collection.h"

#pragma once

class amem : public collection {
	void* data;
	unsigned count;
	unsigned count_maximum;
	unsigned size;
public:
	amem(unsigned size = 0);
	~amem();
	void* add() override;
	char* begin() const { return (char*)data; }
	void clear() override;
	char* end() const { return (char*)data + count*size; }
	void* get(int index) const override;
	unsigned getcount() const override { return count; }
	unsigned getmaxcount() const override { return count_maximum; }
	unsigned getsize() const override { return size; }
	int indexof(const void* element) const override;
	void* insert(int index, const void* object);
	void remove(int index, int count = 1) override;
	void reserve(unsigned count);
	void setup(unsigned size);
};