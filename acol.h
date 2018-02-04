#pragma once

template<class T>
struct acol {
	struct iter {
		T*					current;
		unsigned			size;
		constexpr iter(T* current, unsigned size) : current(current), size(size) {}
		constexpr T&		operator*() const { return *current; }
		constexpr bool		operator!=(const iter& e) const { return e.current != current; }
		constexpr void		operator++() { current = (T*)((char*)current + size); }
	};
	inline iter				begin() const { return iter(pbegin, size); }
	inline iter				end() const { return iter(pend, size); }
	inline T*				get(unsigned index) { return (T*)((char*)pbegin + index*size); }
	unsigned				getcount() { return ((char*)pend - (char*)pbegin) / size; }
	constexpr acol(T* start, unsigned count, unsigned size) : pbegin(start), pend((T*)((char*)start + size*count)), size(size) {}
private:
	T*						pbegin;
	T*						pend;
	unsigned				size;
};
