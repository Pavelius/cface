#pragma once

enum valtype : char
{
	NoValue,
	Value8, Value16, Value32,
	ValueText,
};

struct xsvalue
{
	valtype			type;
	void*			data;
	int				get() const;
	const char*		gets() const;
	void			set(int value);
	unsigned		getsize() const;
};
