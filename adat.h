#pragma once

// Use when we don't want use allocator and must have static data
// Beware - it's not have constructors and destructor
template<class T, unsigned count_max = 128>
struct adat {
	T data[count_max];
	unsigned count;

	const T& operator[](unsigned index) const { return data[index]; }
	T& operator[](unsigned index) { return data[index]; }
	operator bool() const { return count != 0; }

	// Add new element to collection
	T* add() {
		if(count < count_max)
			return data + (count++);
		return 0;
	}
	// Add new element to collection by copy of existing
	void add(const T& e) {
		if(count < count_max)
			data[count++] = e;
	}
	T* begin() {
		return data;
	}
	const T* begin() const {
		return data;
	}
	void clear() {
		count = 0;
	}
	T* end() {
		return data + count;
	}
	const T* end() const {
		return data + count;
	}
	void initialize() {
		count = 0;
	}
	int getcount() const {
		return count;
	}
	unsigned getmaximum() const {
		return count_max;
	}
	int indexof(const T* e) const {
		if(e >= data && e <= data + count)
			return e - data;
		return -1;
	}
	int	indexof(const T t) const {
		for(unsigned i = 0; i < count; i++)
			if(data[i] == t)
				return i;
		return -1;
	}
	bool is(const T t) const {
		for(unsigned i = 0; i < count; i++)
			if(data[i] == t)
				return true;
		return false;
	}
	// Remove 'remove_sount' elements starting from 'index'
	void remove(int index, int remove_count = 1) {
		if(index < 0)
			return;
		if(index<int(count - 1))
			memcpy(data + index, data + index + 1, sizeof(data[0])*(count - index - 1));
		count--;
	}
};