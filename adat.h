#include "initializer_list.h"

#pragma once

// Use when we don't want use allocator and must have static data
// Beware - it's not have constructors and destructor
template<class T, unsigned count_max = 128>
struct adat {
	T data[count_max];
	unsigned count;

	constexpr adat() : data(), count(0) {}
	constexpr adat(std::initializer_list<T> list) : count(0) { for(auto& e : list) *add() = e; }
	constexpr const T& operator[](unsigned index) const { return data[index]; }
	constexpr T& operator[](unsigned index) { return data[index]; }
	operator bool() const { return count != 0; }

	// Add new element to collection
	constexpr T* add() {
		return (count < getmaximum()) ? data + (count++) : data;
	}
	// Add new element to collection by copy of existing
	void add(const T& e) {
		*add() = e;
	}
	constexpr T* begin() {
		return data;
	}
	constexpr const T* begin() const {
		return data;
	}
	void clear() {
		count = 0;
	}
	constexpr T* end() {
		return data + count;
	}
	constexpr const T* end() const {
		return data + count;
	}
	// Get count of elements
	constexpr int getcount() const {
		return count;
	}
	// Get maximum count of elements
	constexpr unsigned getmaximum() const {
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
	// Remove 'remove_count' elements starting from 'index'
	void remove(int index, int remove_count = 1) {
		if(index < 0)
			return;
		if(index<int(count - 1))
			memcpy(data + index, data + index + 1, sizeof(data[0])*(count - index - 1));
		count--;
	}
};