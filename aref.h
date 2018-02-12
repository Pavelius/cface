#pragma once

// Simplest array - pointer and count. Used only for POD types.
template<class T>
struct aref {
	T* data;
	unsigned count;

	T& operator[](int index) { return data[index]; }

	T* add() {
		return data + (count++);
	}

	void add(const T& e) {
		data[count++] = e;
	}

	T* begin() {
		return data;
	}

	const T* begin() const {
		return data;
	}

	void initialize() {
		data = 0; count = 0;
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

	int indexof(const T* t) const {
		if(t<data || t>data + count)
			return -1;
		return t - data;
	}

	int indexof(const T t) const {
		for(int i = 0; i < count; i++)
			if(data[i] == t)
				return i;
		return -1;
	}

	// Remove elements_count from array starting from index
	void remove(int index, int elements_count = 1) {
		if(index < 0 || index >= count)
			return;
		count -= elements_count;
		if(index >= count)
			return;
		memmove(data + index, data + index + elements_count, sizeof(data[0])*(count - index));
	}

};
#define AREF(t) {t, sizeof(t)/sizeof(t[0])}