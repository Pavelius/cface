#include "crt.h"
#include "io.h"
#include "bsdata.h"

//void xsref_write(io::stream& stream, const xsref& e);
//bool xsref_read(io::stream& stream, xsref& e, char* temp);
//bool xsref_read_base(io::stream& stream, xsref& e, char* temp);
//
//template<> void io::stream::write<xsref>(const xsref& e)
//{
//	if(!e)
//		return;
//	auto xs = xsbase::find(e.fields);
//	if(!xs)
//		return;
//	write(xs->id); // Name of metadata
//	write(xs->indexof(e.object)); // Index in metadata or -1
//	xsref_write(*this, e);
//}
//
//template<> void io::stream::write<xsglobal>(const xsglobal& e)
//{
//	if(!e)
//		return;
//	write(e.id);
//	xsref_write(*this, e);
//}
//
//template<> void io::stream::write<xsbase>(const xsbase& m)
//{
//	for(unsigned i = 0; i < m.count; i++)
//		write(m.getref(i));
//}
//
//bool xsbase::write(const char* url, const char** names)
//{
//	io::file file(url, StreamWrite);
//	if(!file)
//		return false;
//	for(int i = 0; names[i]; i++)
//	{
//		auto xs = find(names[i]);
//		if(!xs)
//			continue;
//		for(unsigned n = 0; n < xs->count; n++)
//		{
//			xsref e = {xs->fields, (void*)xs->get(n)};
//			file.stream::write(e);
//		}
//	}
//	return true;
//}
//
//bool xsbase::write(const char* url)
//{
//	io::file file(url, StreamWrite);
//	if(!file)
//		return false;
//	for(auto xs = xsglobal::first; xs; xs = xs->next)
//		file.stream::write(*xs);
//	for(auto xs = first; xs; xs = xs->next)
//	{
//		for(unsigned n = 0; n < xs->count; n++)
//		{
//			xsref e = {xs->fields, (void*)xs->get(n)};
//			file.stream::write(e);
//		}
//	}
//	return true;
//}
//
//void xsref_write(io::stream& stream, const xsref& e)
//{
//	for(auto f = e.fields; *f; f++)
//	{
//		for(unsigned i = 0; i < f->count; i++)
//		{
//			if(f->type == number_type)
//				stream.write(f->ptr(e.object, i), f->size);
//			else if(f->type == text_type)
//			{
//				auto value = (const char*)e.get(f->id, i);
//				stream.write(value);
//			}
//			else if(!f->reference)
//			{
//				auto e1 = e.getr(f->id, i);
//				xsref_write(stream, e1);
//			}
//			else
//			{
//				auto object = (void*)e.get(f->id, i);
//				auto xs = xsbase::findbyptr(object);
//				if(!xs)
//					stream.write(0);
//				else
//				{
//					stream.write(xs->id);
//					if(xs->id[0])
//						stream.write(xs->indexof(object));
//				}
//			}
//		}
//	}
//}
//
//template<> void io::stream::read<xsref>(xsref& e)
//{
//	char temp[xsfield_max_text];
//	xsref_read_base(*this, e, temp);
//}
//
//bool bsfield_bin_read_single(io::stream& stream, void* object, const bsreq* fields, char* temp)
//{
//	for(auto f = fields; *f; f++)
//	{
//		for(unsigned i = 0; i < f->count; i++)
//		{
//			if(f->type == number_type)
//				stream.read((void*)f->ptr(e.object, i), f->size);
//			else if(f->type == text_type)
//			{
//				stream.readtext(temp, bsreq_max_text - 1);
//				if(temp[0])
//					e.set(f->id, (int)szdup(temp), i);
//				else
//					e.set(f->id, 0, i);
//			}
//			else if(!f->reference)
//			{
//				if(!bsfield_bin_read_single(stream, (void*)f->ptr(object, i), f->type, temp))
//					return false;
//			}
//			else
//			{
//				int index = 0;
//				stream.readtext(temp, xsfield_max_text - 1);
//				if(temp[0])
//					stream.read(index);
//				auto xs = xsbase::find(temp);
//				if(xs)
//					e.set(f->id, (int)xs->get(index), i);
//				else
//					e.set(f->id, 0, i);
//			}
//		}
//	}
//	return true;
//}
//
//bool bsfield_bin_read(io::stream& stream, void* object, const bsreq* e, char* temp)
//{
//	stream.readtext(temp, bsreq_max_text - 1);
//	//auto gs = xsglobal::find(temp);
//	//if(gs)
//	//{
//	//	e.fields = gs->fields;
//	//	e.object = gs->object;
//	//	return xsref_read(stream, *gs, temp);
//	//}
//	auto pd = bsdata::find(temp);
//	if(pd)
//	{
//		void* object = 0;
//		int index = -1;
//		stream.read(index);
//		if(index<0)
//			object = pd->add();
//		else
//		{
//			if((unsigned)index < pd->getmaxcount() && (unsigned)index >= pd->getcount())
//				pd->setcount(index+1);
//			object = (void*)pd->get(index);
//		}
//		return xsref_read(stream, e, temp);
//	}
//	return false;
//}