#include "collection.h"
#include "crt.h"
#include "io_plugin.h"
#include "bsdata.h"

const int xsfield_max_text = 8192;

bool xsref_read(io::stream& stream, xsref& e, char* temp);
void xsref_write(io::stream& stream, const xsref& e);

void collection_read(io::stream& stream, collection& col, const bsreq* fields) {
	char temp[xsfield_max_text];
	int count = 0;
	col.clear();
	stream.read(count);
	for(int i = 0; i < count; i++) {
		xsref e = {fields, col.add(0)};
		xsref_read(stream, e, temp);
	}
}

void collection_write(io::stream& stream, collection& col, const bsreq* fields) {
	int count = col.getcount();
	stream.write(count);
	for(int i = 0; i < count; i++)
		xsref_write(stream, {fields, col.get(i)});
}

struct collection_reader : public io::reader {
	collection&		tb;
	xsref			row;

	bool isnumeric(const char* value) const {
		return value[0] == '-' || isnum(value[0]);
	}

	void open(io::reader::node& e) override {
		if(e.parent && e.parent->parent == 0)
			row.object = tb.add(0);
	}

	void set(io::reader::node& e, const char* value) override {
		if(!row)
			return;
		if(!e.parent)
			return;
		if(*e.parent=="element") {
			if(isnumeric(value))
				row.set(e.name, sz2num(value));
			else
				row.set(e.name, (int)szdup(value));
		}
	}

	collection_reader(collection& tb, const xsfield* fields) :tb(tb) {
		row.fields = fields;
		row.object = 0;
	}

};

bool collection::read(const char* url, bsreq* fields) {
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

bool collection::write(const char* url, bsreq* fields) {
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
			xsref xr = {fields, get(i)};
			for(auto p = fields; *p; p++) {
				auto v = xr.get(p->id);
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