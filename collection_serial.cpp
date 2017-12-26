#include "collection.h"
#include "crt.h"
#include "io_plugin.h"
#include "bsdata.h"

//bool bsfield_bin_read(io::stream& stream, void* object, const bsreq* e, char* temp);
void write_fields(io::stream& e, const void* object, const bsreq* req, const bsreq* skip);

void collection_read(io::stream& stream, collection& col, const bsreq* fields) {
	int count = 0;
	col.clear();
	stream.read(count);
	//for(int i = 0; i < count; i++)
	//	bsfield_bin_read(stream, col.add(0), fields, temp);
}

void collection_write(io::stream& stream, collection& col, const bsreq* fields) {
	int count = col.getcount();
	stream.write(count);
	for(int i = 0; i < count; i++)
		write_fields(stream, col.get(i), fields, 0);
}

struct collection_reader : public io::reader {
	
	collection&		tb;
	void*			object;
	const bsreq*	fields;

	bool isnumeric(const char* value) const {
		return value[0] == '-' || isnum(value[0]);
	}

	void open(io::reader::node& e) override {
		if(e.parent && e.parent->parent == 0)
			object = tb.add(0);
	}

	void set(io::reader::node& e, const char* value) override {
		if(!object)
			return;
		if(!e.parent)
			return;
		auto f = fields->find(e.name);
		if(f) {
			if(*e.parent == "element") {
				if(isnumeric(value))
					f->set(f->ptr(object), sz2num(value));
				else
					f->set(f->ptr(object), (int)szdup(value));
			}
		}
	}

	collection_reader(collection& tb, const bsreq* fields) :tb(tb), object(0), fields(fields) {
	}

};

bool collection::read(const char* url, const bsreq* fields) {
	auto ex = szext(url);
	auto pp = io::plugin::find(ex);
	if(pp) {
		collection_reader r(*this, fields);
		const char* p = loadt(url);
		if(!p)
			return false;
		const char* p1 = pp->read(p, r);
		delete p;
	} else if(strcmp(ex, "dat") == 0 || strcmp(ex, "bin") == 0) {
		io::file file(url, StreamRead);
		if(!file)
			return false;
		collection_read(file, *this, fields);
	} else
		return false;
	return true;
}

bool collection::write(const char* url, const bsreq* fields) {
	auto ex = szext(url);
	if(!ex)
		return false;
	auto pp = io::plugin::find(ex);
	if(pp) {
		io::file file(url, StreamWrite | StreamText);
		if(!file)
			return false;
		int count = getcount();
		io::writer* pw = pp->write(file);
		if(!pw)
			return false;
		pw->open("rows", io::Array);
		for(int i = 0; i < count; i++) {
			pw->open("element", io::Struct);
			void* object = get(i);
			for(auto p = fields; *p; p++) {
				auto v = p->get(p->ptr(object));
				if(!v)
					continue;
				if(p->type == number_type)
					pw->set(p->id, v, io::Number);
				else if(p->type == text_type)
					pw->set(p->id, (const char*)v, io::Text);
			}
			pw->close("element", io::Struct);
		}
		pw->close("rows", io::Array);
		delete pw;
	} else if(strcmp(ex, "dat") == 0 || strcmp(ex, "bin") == 0) {
		io::file file(url, StreamWrite);
		if(!file)
			return false;
		collection_write(file, *this, fields);
	} else
		return false;
	return true;
}