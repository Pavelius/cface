#include "initializer_list.h"

#pragma once

// Abstract flag data bazed on enumerator
template<typename T, typename DT = unsigned> class cflags {
	static constexpr T maximum = (T)(sizeof(DT) * 8);
	struct iter {
		T	current;
		DT	data;
		iter(T current, DT data) : current(current), data(data) {}
		T operator*() const { return (T)current; }
		bool operator!=(const iter& e) const { return e.current != current; }
		void operator++() {
			while(current < maximum) {
				current = (T)(current + 1);
				if((data & (1 << current)) != 0)
					break;
			}
		}
	};
	static constexpr unsigned gen(unsigned r, const T* ps, const T* pe) {
		return (ps < pe) ? gen(r | (1 << (*ps)), ps + 1, pe) : r;
	}
	constexpr unsigned getcount(unsigned r) const {
		return (r < (sizeof(data) * 8)) ? getcount(r + 1) + ((data & (1 << r)) ? 1 : 0) : 0;
	}
public:
	DT			data;
	constexpr cflags() : data(0) {}
	constexpr cflags(std::initializer_list<T> list) : data(gen(0, list.begin(), list.end())) {}
	void		add(T id) { data |= 1 << id; }
	iter		begin() const { return iter((T)0, data); }
	void		clear() { data = 0; }
	iter		end() const { return iter(maximum, data); }
	constexpr unsigned getcount() const { return getcount(0) }
	bool		is(T id) const { return (data & (1 << id)) != 0; }
	void		remove(T id) { data &= ~(1 << id); }
	void		set(T id) { data |= 1 << id; }
};