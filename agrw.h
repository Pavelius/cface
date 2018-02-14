#pragma once

// Auto grow array for custom allocators.
template<typename T, int max_count = 4096>
struct agrw {
	unsigned count;
	T data[max_count];
	agrw* next;
	constexpr agrw() : next(0), count(0) {}
	T* add() {
		auto t = this;
		while(t->count >= max_count) {
			if(!t->next) t->next = new agrw();
			t = t->next;
		}
		return t->data + t->count++;
	}
	T* begin() {
		return data; 
	}
	T* end() {
		return data + count;
	}
	void clear() {
		auto z = next;
		while(z) {
			auto n = z->next;
			z->next = 0;
			delete z;
			z = n;
		}
		next = 0;
	}
	unsigned getcount() const {
		unsigned result = 0;
		for(auto p = this; p; p = p->next)
			result += p->count;
		return result;
	}
	int indexof(const T* value) const {
		auto result = 0;
		for(auto p = this; *p; p = p->next) {
			if(value >= p->data && value <= p->data[p->count - 1])
				return result + (value - p->data);
		}
		return -1;
	}
};