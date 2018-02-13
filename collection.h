#pragma once

struct collection {
	virtual void* add() = 0; // Add new empthy element to collection
	void add(const void* element); // Add new element to collection
	virtual void clear() = 0; // Clear all elements in collection
	int find(const char* value, unsigned offset = 0); // Find value by name
	virtual void* get(int index) const = 0; // Get content of element with index
	virtual unsigned getmaxcount() const = 0; // Get maximum possible elements
	virtual unsigned getcount() const = 0; // Get count of elements in collection
	virtual unsigned getsize() const = 0; // Get size of one element in collection
	virtual int indexof(const void* element) const = 0; // Get index of element (-1 if not in collection)
	bool read(const char* url, const struct bsreq* fields);
	virtual void remove(int index, int count = 1) = 0; // Remove element from collection
	void sort(int i1, int i2, int(*compare)(const void* p1, const void* p2, void* param), void* param);	// Sort collection
	void swap(int i1, int i2); // Swap elements
	bool write(const char* url, const struct bsreq* fields);
};