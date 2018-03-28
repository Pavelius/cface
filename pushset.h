#pragma once

template<class T> class pushset {
	T&	variable;
	T	variable_value;
public:
	pushset(T& variable, T value) : variable(variable), variable_value(variable) { variable = value; }
	~pushset() { variable = variable_value; }
};
