#pragma once

enum ctype_flags {
	TypeNamespace, TypeModule, TypeClass, TypeBinary, TypeMethod, TypeField, TypeConst,
	TypeCompileMask = 0x0F,
	TypePrivate = 0x10,
	TypeStatic = 0x20,
};
struct ctype
{
	const char*			name;
	int					count;
	int					size;
	int					value; // Offset or constant value
	int					position; // Position in text file
	unsigned			flags; // Some flags from ctype_flags
	ctype*				child; // Children requisites
	ctype*				result; // Type result (like 'int' in 'int x')
	ctype*				next;
	//
	static ctype		root;
	static ctype		pointers;
	static ctype		charu;
	static ctype		chars;
	static ctype		charp;
	static ctype		ints;
	static ctype		intu;
	static ctype		intss;
	static ctype		intsu;
	static ctype		boolx;
	static ctype		voidx;
	static ctype		voidp;
	//
	operator bool() const { return name!=0; }
	void*				operator new(unsigned size);
	void				operator delete(void* p);
	//
	ctype*				add(unsigned token, const char* name, ctype& result = voidx);
	void				clear();
	ctype*				dereference() const;
	ctype*				find(const char* name);
	ctype_flags			gtype() const { return ctype_flags(flags&TypeCompileMask);  }
	static void			initialize();
	ctype*				reference() const;
	char*				tostring(char* buffer) const;
};