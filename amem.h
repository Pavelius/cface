#include "collection.h"

#pragma once

struct amem : collection
{
	void*				data;
	unsigned			count;
	unsigned			count_maximum;
	unsigned			size;
	amem(unsigned size = 0);
	~amem();
	void*				add(const void* element = 0) override;
	char*				begin() const;
	void				clear() override;
	char*				end() const { return begin() + count*size; }
	void*				get(int index) const override;
	unsigned			getcount() const override;
	int					indexof(const void* element) const override;
	void*				insert(int index, const void* object);
	void				remove(int index, int count = 1) override;
	bool				reserve(unsigned count);
	void				setup(unsigned size);
	void				swap(int i1, int i2) override; // Swap elements
};