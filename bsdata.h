#include "bsreq.h"
#include "collection.h"

#pragma once

struct bsdata : collection
{
	const char*			id;
	const bsreq*		fields;
	void*				data;
	unsigned			size;
	bsdata*				next;
	static bsdata*		first;
	//
	bsdata(const char* id, void* data, unsigned size, unsigned maximum_count, const bsreq* fields);
	bsdata(const char* id, void* data, unsigned& count, unsigned size, unsigned maximum_count, const bsreq* fields);
	//
	void*				add(const void* element = 0) override;
	char*				begin() const { return (char*)data; }
	void				clear() override { count = 0; }
	char*				end() const { return (char*)data + size*count; }
	void*				get(int index) const override { return (char*)data + index*size; }
	unsigned			getcount() const override { return count; }
	unsigned			getmaxcount() const override { return maximum_count; }
	int					indexof(const void* element) const override;
	static bsdata*		find(const char* id);
	static bsdata*		find(const bsreq* id);
	void*				find(const bsreq* id, const char* value);
	static bsdata*		findbyptr(const void* object);
	static void			parse(const char* value);
	static void			read(const char* url);
	void				remove(int index, int count = 1) override;
	void				setcount(unsigned value) { count = value; }
	void				swap(int i1, int i2) override;
	static void			write(const char* url, const char** baseids);
	static void			write(const char* url, const char* baseid);
private:
	unsigned			maximum_count;
	unsigned			current_count;
	unsigned&			count;
};
#define BSMETA(c) \
bsdata c##_manager(#c, c##_data, sizeof(c##_data[0]), sizeof(c##_data)/sizeof(c##_data[0]), c##_type);
#define BSGLOB(c) \
bsdata c##_manager(#c, &c##_data, sizeof(c##_data), 1, c##_type);
#define BSDATA(c, c_type) \
bsdata c##_manager(#c, c##s.data, c##s.count, sizeof(c##s.data[0]), sizeof(c##s.data)/sizeof(c##s.data[0]), c_type);