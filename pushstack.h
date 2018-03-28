#pragma once

template<class T> class pushstack {
	T&	value;
	T	push_value;
public:
	pushstack(T& value) : value(value), push_value(value) {}
	~pushstack() { value = push_value; }
};
