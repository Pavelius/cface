#include "crt.h"
#include "io.h"
#include "xsbase.h"
#include "xsglobal.h"

void xsref_write(io::stream& stream, const xsref& e);
bool xsref_read(io::stream& stream, xsref& e, char* temp);
bool xsref_read_base(io::stream& stream, xsref& e, char* temp);

template<> void io::stream::write<xsref>(const xsref& e)
{
	if(!e)
		return;
	auto xs = xsbase::find(e.fields);
	if(!xs)
		return;
	puttext(xs->id); // Name of metadata
	write(xs->indexof(e.object)); // Index in metadata or -1
	xsref_write(*this, e);
}

template<> void io::stream::write<xsglobal>(const xsglobal& e)
{
	if(!e)
		return;
	puttext(e.id);
	xsref_write(*this, e);
}

template<> void io::stream::write<xsbase>(const xsbase& m)
{
	for(unsigned i = 0; i < m.count; i++)
		write(m.getref(i));
}

bool xsbase::write(const char* url, const char** names)
{
	io::file file(url, StreamWrite);
	if(!file)
		return false;
	for(int i = 0; names[i]; i++)
	{
		auto xs = find(names[i]);
		if(!xs)
			continue;
		for(unsigned n = 0; n < xs->count; n++)
		{
			xsref e = {xs->fields, (void*)xs->get(n)};
			file.stream::write(e);
		}
	}
	return true;
}

bool xsbase::write(const char* url)
{
	io::file file(url, StreamWrite);
	if(!file)
		return false;
	for(auto xs = xsglobal::first; xs; xs = xs->next)
		file.stream::write(*xs);
	for(auto xs = first; xs; xs = xs->next)
	{
		for(unsigned n = 0; n < xs->count; n++)
		{
			xsref e = {xs->fields, (void*)xs->get(n)};
			file.stream::write(e);
		}
	}
	return true;
}

void xsref_write(io::stream& stream, const xsref& e)
{
	for(auto f = e.fields; *f; f++)
	{
		for(unsigned i = 0; i < f->count; i++)
		{
			if(f->type == number_type)
				stream.write(f->ptr(e.object, i), f->size);
			else if(f->type == text_type)
			{
				auto value = (const char*)e.get(f->id, i);
				stream.puttext(value);
			}
			else if(!f->reference)
			{
				auto e1 = e.getr(f->id, i);
				xsref_write(stream, e1);
			}
			else
			{
				auto object = (void*)e.get(f->id, i);
				auto xs = xsbase::findbyptr(object);
				if(!xs)
					stream.write(0);
				else
				{
					stream.puttext(xs->id);
					if(xs->id[0])
						stream.write(xs->indexof(object));
				}
			}
		}
	}
}

template<> void io::stream::read<xsref>(xsref& e)
{
	char temp[xsfield_max_text];
	xsref_read_base(*this, e, temp);
}

bool xsref_read(io::stream& stream, xsref& e, char* temp)
{
	for(auto f = e.fields; *f; f++)
	{
		for(unsigned i = 0; i < f->count; i++)
		{
			if(f->type == number_type)
				stream.read((void*)f->ptr(e.object, i), f->size);
			else if(f->type == text_type)
			{
				stream.gettext(temp, xsfield_max_text - 1);
				if(temp[0])
					e.set(f->id, (int)szdup(temp), i);
				else
					e.set(f->id, 0, i);
			}
			else if(!f->reference)
			{
				xsref e1 = {f->type, (void*)f->ptr(e.object, i)};
				if(!xsref_read(stream, e1, temp))
					return false;
			}
			else
			{
				int index = 0;
				stream.gettext(temp, xsfield_max_text - 1);
				if(temp[0])
					stream.read(index);
				auto xs = xsbase::find(temp);
				if(xs)
					e.set(f->id, (int)xs->get(index), i);
				else
					e.set(f->id, 0, i);
			}
		}
	}
	return true;
}

bool xsref_read_base(io::stream& stream, xsref& e, char* temp)
{
	stream.gettext(temp, xsfield_max_text - 1);
	auto gs = xsglobal::find(temp);
	if(gs)
	{
		e.fields = gs->fields;
		e.object = gs->object;
		return xsref_read(stream, *gs, temp);
	}
	auto xs = xsbase::find(temp);
	if(xs)
	{
		e.fields = xs->fields;
		int index = -1;
		stream.read(index);
		if(index<0)
			e.object = xs->add();
		else
		{
			if((unsigned)index < xs->count_maximum && (unsigned)index >= xs->count)
				xs->count = index+1;
			e.object = (void*)xs->get(index);
		}
		return xsref_read(stream, e, temp);
	}
	return false;
}

bool xsbase::read(const char* url)
{
	io::file file(url, StreamRead);
	if(!file)
		return false;
	xsref e = {0, 0};
	char temp[xsfield_max_text];
	int count = 0;
	while(xsref_read_base(file, e, temp))
		count++;
	return true;
}