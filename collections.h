#include "adat.h"
#include "aref.h"
#include "collection.h"

#pragma once

template<class T, int count_max = 128>
struct adatc : adat<T, count_max>, collection
{
	void					add(const T& e) { adat::add(e); }
	void*					add(const void* element = 0) override { if(count < count_max) data[count] = (T&)element; return data + count++; }
	void					clear() override { adat::clear(); }
	void*					get(int index) const override { return (T*)data + index; }
	unsigned				getcount() const { return adat::getcount(); }
	int						indexof(const void* element) const override { return adat::indexof((T*)element); }
	void					remove(int index, int count = 1) override { adat::remove(index, count); }
	void					swap(int i1, int i2) { adat::swap(i1, i2); }
};
template <class T>
struct arefc : aref<T>, collection
{
	arefc() { initialize(); }
	~arefc() { aref<T>::clear(); }
	void*					add(const void* object) override { auto& m = addr(); if(object) m = *static_cast<const T*>(object); return &m; }
	void					clear() { aref<T>::clear(); }
	unsigned				getcount() const override { return count; }
	void*					get(int index) const override { return data + index; }
	int						indexof(const void* object) const override { return aref<T>::indexof(static_cast<const T*>(object)); }
	void					remove(int index, int elements_count) override { aref<T>::remove(index, elements_count); }
	void					swap(int i1, int i2) override { T a = data[i1]; data[i1] = data[i2]; data[i2] = a; }
};
