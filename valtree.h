#pragma once

struct valtree
{
	enum types {Number, Text, Structure};
	types			type;
	const char*		name;
	valtree*		value;
	valtree*		next;
	//
	valtree();
	~valtree();
	//
	void* operator new(unsigned size);
	const valtree*	find(const char* name) const;
	const char*		gets(const char* name) const;
	int				geti(const char* name) const;
	bool			read(const char* url);
	valtree&		set(const char* name) { return add(name, Structure, 0); }
	valtree&		set(const char* name, const char* value) { return add(name, Text, (valtree*)value); }
	valtree&		set(const char* name, int value) { return add(name, Number, (valtree*)value); }
	void			set(types type);
	void			write(const char* url);
private:
	valtree&		add(const char* name, valtree::types type, valtree* value);
};