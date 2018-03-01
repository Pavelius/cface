#pragma once

void* rmreserve(void* ptr, unsigned size);
unsigned rmoptimal(unsigned need_count);

template <class T1, class T2>
class amap {
	struct element {
		T1 key;
		T2 value;
	};
	element* data;
	unsigned count;
	unsigned count_maximum;
	void reserve() {
		if(data && count_maximum >= count)
			return;
		count_maximum = rmoptimal(count + 1);
		data = (element*)rmreserve(data, count_maximum*sizeof(element));
	}
public:
	typedef T1 key_type;
	typedef T2 value_type;
	constexpr amap() : data(0), count(0) {}
	~amap() { if(data) rmreserve(data, 0); }
	constexpr element* begin() { return data; }
	constexpr element* end() { return data + count; }
	T2* add(T1 k) {
		reserve();
		auto p = data + (count++);
		p->key = k;
		return &p->value;
	}
	T2* find(T1 k) {
		for(auto& e : *this) {
			if(e.key == k)
				return &e.value;
		}
		return 0;
	}
};