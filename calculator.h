#pragma once

enum calc_errors {
	CalculatorNoError,
	CalculatorNotFound,
};

// Simple script executor
struct calculator
{
	struct error
	{
		calc_errors	type;
		const char*	position;
		const char*	expected;
		operator bool() const { return type != CalculatorNoError ; }
	};
	struct var
	{
		const char*	name;
		int			value;
	};
	error			errors; // When error occurs, there is filed position
	calculator*		parent; // Parent context. In function calling you can analize this field and make Call Stack tree.
	var				variables[16]; // Variables that can be used in calculation
	bool			stop; // Not do any calculation (but parse anyway), do not call functions
	//
	static int		getconstant(const char* name); // Get constant value. Must be overrided in application.
	static int		getfunction(const char* name, calculator& e, calculator& parameters, int parameters_count); // Get function. Must be overrided in application.
	static int		getindirection(int e1, const char* name);
	//
	calculator();
	inline operator bool() const { return !errors; }
	//
	void			add(const char* name, int value);
	void			clear();
	int				evalute(const char* string); // Evalute code expression and return result
	var*			find(const char* name);
};