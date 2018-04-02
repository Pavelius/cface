#pragma once

template<class T>
class acol {
	class iter {
		T* current;
		unsigned size;
	public:
		constexpr iter(T* current, unsigned size) : current(current), size(size) {}
		constexpr T& operator*() const { return *current; }
		constexpr bool operator!=(const iter& e) const { return e.current != current; }
		constexpr void operator++() { current = (T*)((char*)current + size); }
	};
	T* pbegin;
	T* pend;
	unsigned size;
public:
	constexpr acol(T* start, unsigned count, unsigned size) : pbegin(start), pend((T*)((char*)start + size * count)), size(size) {}
	operator bool() const { pbegin != pend; }
	iter begin() const { return iter(pbegin, size); }
	iter end() const { return iter(pend, size); }
	T* get(unsigned index) { return (T*)((char*)pbegin + index * size); }
	unsigned getcount() const { return ((char*)pend - (char*)pbegin) / size; }
};