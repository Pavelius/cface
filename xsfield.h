#pragma once

const int xsfield_max_text = 8192 * 2; // Maximum text for field (for serialization)

// Metadata field descriptor
struct xsfield
{
	template<class T> struct info
	{
		static constexpr int size = sizeof(T);
		static constexpr int count = 1;
	};
	template<class T, unsigned N> struct info<T[N]>
	{
		static constexpr int size = sizeof(T);
		static constexpr int count = N;
	};
	template<class T> struct info<T[]>
	{
		static constexpr int size = sizeof(T);
		static constexpr int count = 0;
	};
	template<class T> struct refi
	{
		static constexpr int count = 0;
	};
	template<class T> struct refi<T*>
	{
		static constexpr int count = 1 + refi<T>::count;
	};
	template<class T, int N> struct refi<T[N]>
	{
		static constexpr int count = refi<T>::count;
	};
	template<class T> struct refi<T[]>
	{
		static constexpr int count = refi<T>::count;
	};
	const char*			id; // field identifier
	unsigned			offset; // offset from begin of class or object
	unsigned			size; // size of single element
	unsigned			lenght; // total size in bytes of all field (array has size*count)
	unsigned			count; // count of elements
	unsigned char		reference; // 1+ is reference
	const xsfield*		type; // metadata of element
	//
	operator bool() const { return id != 0; }
	//
	const xsfield*		find(const char* name) const;
	int					get(const void* p) const;
	bool				issimple() const { return type == 0; }
	bool				iskey() const { return id[0] == 'i' && id[1] == 'd' && id[2] == 0; }
	inline const char*	ptr(const void* data) const { return (const char*)data + offset; }
	inline const char*	ptr(const void* data, int index) const { return (const char*)data + offset + index*size; }
	void				set(const void* p, int value) const;
};

extern xsfield			number_type[2]; // standart integer value
extern xsfield			text_type[2]; // stantart zero ending string
extern xsfield			reference_type[2]; // any reference (depends on value)
extern xsfield			xsfield_type[]; // metadata type

#define PROPERTY(name, type, def)\
virtual type get##name() const { return def; }\
virtual void set##name(type value) {}
#define FO(c,f) (const int)&((c*)0)->f
#define	BSREQ(cls, field, type) {#field, FO(cls,field), xsfield::info<decltype(cls::field)>::size, sizeof(cls::field), xsfield::info<decltype(cls::field)>::count, xsfield::refi<decltype(cls::field)>::count, type}
#define	BSVAR(field, type) {#field, (unsigned)&field, xsfield::info<decltype(field)>::size, sizeof(field), xsfield::info<decltype(field)>::count, xsfield::refi<decltype(field)>::count, type}