#include "crt.h"
#include "bsdata.h"
#include "io.h"

static const bsreq* getkeyreq(const bsreq* type) {
	auto f = type->find("id");
	if(!f || f->type != text_type)
		f = type->find("name");
	if(f->type != text_type)
		return 0;
	return f;
}

struct bsparse {

	char buffer[128 * 256];
	int value;
	const bsreq* value_type;
	void* value_object;
	void* parent_object;
	const bsreq* parent_type;
	const char *p;

	bsparse(const char* source) : p(source) {
		clearvalue();
		buffer[0] = 0;
	}

	void clearvalue() {
		value = 0;
		value_type = 0;
		value_object = 0;
	}

	bool islinefeed() const {
		return *p == 13 || *p == 10;
	}

	void skip() {
		p++;
	}

	bool skip(char sym) {
		if(*p != sym)
			return false;
		p++;
		skipws();
		return true;
	}

	bool skip(const char* sym) {
		auto i = 0;
		while(sym[i]) {
			if(p[i] != sym[i])
				return false;
			i++;
		}
		p += i;
		return true;
	}

	void skipws() {
		while(*p) {
			if(p[0] == 9 || p[0] == 0x20) {
				p++;
				continue;
			} else if(p[0] == '\\') {
				p++;
				if(p[0] == 10 || p[0] == 13)
					p = szskipcr(p);
				else
					p++;
				continue;
			} else if(p[0] == '/' && p[1] == '/') {
				// Comments
				p += 2;
				skipline();
				continue;
			}
			break;
		}
	}

	void skipline() {
		while(p[0] && p[0] != 10 && p[0] != 13)
			p++;
		p = szskipcr(p);
		skipws();
	}

	void error() {
		skipline();
	}

	void readstring(const char end) {
		auto ps = buffer;
		auto pe = ps + sizeof(buffer) / sizeof(buffer[0]) - 1;
		for(; p[0] && p[0] != end; p++) {
			if(p[0] == '\\')
				continue;
			if(ps < pe)
				*ps++ = p[0];
		}
		if(p[0] == end)
			p++;
		ps[0] = 0;
	}

	bool readidentifier() {
		if((p[0] >= 'a' && p[0] <= 'z') || (p[0] >= 'A' && p[0] <= 'Z')) {
			auto ps = buffer;
			auto pe = ps + sizeof(buffer) / sizeof(buffer[0]) - 1;
			while(*p && ((*p >= '0' && *p <= '9') || *p == '_' || ischa(*p))) {
				if(ps < pe)
					*ps++ = *p;
				p++;
			}
			ps[0] = 0;
		} else
			return false;
		return true;
	}

	bool readvalue(const bsreq* hint_type, bool create) {
		bool need_identifier = false;
		buffer[0] = 0;
		value = 0;
		value_type = 0;
		value_object = 0;
		if(p[0] == '-' || (p[0] >= '0' && p[0] <= '9')) {
			value = sz2num(p, &p);
			value_type = number_type;
		} else if(p[0] == '\'') {
			p++;
			readstring('\'');
			value_type = text_type;
			need_identifier = true;
		} else if(p[0] == '\"') {
			p++;
			readstring('\"');
			value_type = text_type;
		} else if(readidentifier()) {
			value_type = text_type;
			need_identifier = true;
		} else
			return false; // Not found value tag
		if(need_identifier) {
			auto value_data = bsdata::find(hint_type);
			if(!value_data) {
				for(value_data = bsdata::first; value_data; value_data = value_data->next) {
					auto f = getkeyreq(value_data->fields);
					if(!f)
						continue;
					value_object = value_data->find(f, buffer);
					if(value_object)
						break;
				}
			} else
				value_object = value_data->find(getkeyreq(value_data->fields), buffer);
			// If not find create this
			if(!value_object && value_data && create) {
				auto f = getkeyreq(value_data->fields);
				if(f) {
					value_object = value_data->add();
					f->set(f->ptr(value_object), (int)szdup(buffer));
				}
			}
			if(value_data)
				value_type = value_data->fields;
			else
				value_type = reference_type;
			if(value_object && value_data)
				value = value_data->indexof(value_object);
		} else if(create && hint_type && value_type == number_type) {
			auto value_data = bsdata::find(hint_type);
			value_type = hint_type;
			if(value_data) {
				if(value < (int)value_data->getmaxcount()) {
					if(value >= (int)value_data->getcount())
						value_data->setcount(value + 1);
					value_object = value_data->get(value);
				}
			}
		}
		skipws();
		return true;
	}

	void storevalue(void* object, const bsreq* req, unsigned index) {
		if(!object || !req)
			return;
		if(index)
			object = (char*)object + req->size*index;
		if(req->type == text_type) {
			if(!buffer[0])
				req->set(req->ptr(object), 0);
			else
				req->set(req->ptr(object), (int)szdup(buffer));
		} else if(req->type == number_type)
			req->set(req->ptr(object), value);
		else if(req->type->reference)
			req->set(req->ptr(object), (int)value_object);
	}

	bool readreq(void* object, const bsreq* req, unsigned index) {
		if(!skip('('))
			return false;
		while(*p) {
			if(skip(')'))
				break;
			readvalue(0, false);
			storevalue(object, req, index);
			if(skip(','))
				index++;
		}
		skipws();
		return true;
	}

	bool readfields(void* object, const bsreq* fields) {
		while(*p && !islinefeed()) {
			const bsreq* req = 0;
			if(readidentifier())
				req = fields->find(buffer);
			readreq(object, req, 0);
		}
		return true;
	}

	bool readrecord() {
		if(!skip('#'))
			return false;
		// Read data base name
		if(!readidentifier()) {
			skipline();
			return true;
		}
		skipws();
		const bsreq* fields = 0;
		auto pd = bsdata::find(buffer);
		if(pd)
			fields = pd->fields;
		// Read key value
		parent_object = value_object;
		readvalue(fields, true);
		readfields(value_object, fields);
		parent_type = fields;
		return true;
	}

	bool readsubrecord() {
		auto index = 0;
		auto last_field = 0;
		while(skip("##")) {
			// Read data base name
			if(!readidentifier()) {
				error();
				return true;
			}
			skipws();
			auto parent_field = parent_type->find(buffer);
			if(!parent_field) {
				error();
				return true;
			}
			if(parent_field->count <= 1 // Only array may be defined as ##
				|| parent_field->reference // No reference allowed
				|| parent_field->isenum // Enumeratior must be initialized in row
				|| parent_field->type->issimple()) { // No Simple type
				error();
			}
			readfields((void*)parent_field->ptr(parent_object, index),
				parent_field->type);
			index++;
		}
		// If aref or adat save count
		return false;
	}

	void readtrail() {
		auto pb = buffer;
		auto pe = pb + sizeof(buffer) - 1;
		while(true) {
			auto sym = *p;
			if(!sym)
				break;
			if(sym == '\n' || sym == '\r') {
				while(*p == '\n' || *p == '\r') {
					p = szskipcr(p);
					skipws();
				}
				if(*p == '#')
					break;
				if(pb != buffer && pb < pe)
					*pb++ = '\n';
				continue;
			}
			if(pb < pe)
				*pb++ = sym;
			p++;
		}
		*pb = 0;
	}

	void parse() {
		while(*p) {
			if(!readrecord())
				return;
		}
	}

};

static void skipws(io::sequence& e) {
	while(e) {
		if(e.match("\t") || e.match(" ")) {
			p++;
			continue;
		} else if(p[0] == '\\') {
			p++;
			if(p[0] == 10 || p[0] == 13)
				p = szskipcr(p);
			else
				p++;
			continue;
		} else if(p[0] == '/' && p[1] == '/') {
			// Comments
			p += 2;
			skipline();
			continue;
		}
		break;
	}
}

static bool isidentifier(const char* p) {
	if((p[0] >= 'a' && p[0] <= 'z') || (p[0] >= 'A' && p[0] <= 'Z')) {
		while(*p) {
			if(!((*p >= '0' && *p <= '9') || *p == '_' || ischa(*p)))
				return false;
			p++;
		}
		return true;
	}
	return false;
}

static bool isempthy(const void* object, const bsreq* req, bool check_array = true) {
	auto ps = req->ptr(object);
	if(check_array && req->count > 1) {
		for(auto i = 0; i < (int)req->count; i++)
			if(!isempthy((void*)req->ptr(object, i), req, false))
				return false;
	} else {
		for(unsigned i = 0; i < req->size; i++) {
			if(ps[i])
				return false;
		}
		if(req->type == text_type && req->count == 1 && req->reference) {
			auto value = (const char*)req->get(req->ptr(object));
			if(value && value[0] == 0)
				return false;
		}
	}
	return true;
}

static void write_identifier(io::stream& e, const char* value) {
	if(isidentifier(value))
		e << value;
	else
		e << "'" << value << "'";
}

static void write_number(io::stream& e, int value) {
	e << value;
}

static const bsreq* write_key(io::stream& e, const void* object, const bsreq* type) {
	auto f = getkeyreq(type);
	auto v = "";
	if(f)
		v = (const char*)f->get(f->ptr(object));
	if(v && v[0])
		write_identifier(e, v);
	else {
		auto pd = bsdata::findbyptr(object);
		if(!pd)
			return 0;
		write_number(e, pd->indexof(object));
	}
	return f;
}

static void write_value(io::stream& e, const void* object, const bsreq* req, int level);

static void write_array(io::stream& e, const void* object, const bsreq* req, int level) {
	if(level > 0 && req->count > 1)
		e << "(";
	for(unsigned index = 0; index < req->count; index++) {
		if(index > 0)
			e << ", ";
		write_value(e, req->ptr(object, index), req, level);
	}
	if(level > 0 && req->count > 1)
		e << ")";
}

static void write_value(io::stream& e, const void* object, const bsreq* req, int level) {
	if(req->type == number_type) {
		auto value = req->get(object);
		e << value;
	} else if(req->type == text_type) {
		auto value = (const char*)req->get(object);
		e << "\"" << value << "\"";
	} else if(req->reference) {
		auto value = (const void*)req->get(object);
		write_key(e, value, req->type);
	} else if(req->isenum) {
		auto value = req->get(object);
		auto pd = bsdata::find(req->type);
		if(pd)
			write_key(e, pd->get(value), req->type);
		else
			e << value;
	} else {
		if(level > 0)
			e << "(";
		auto count = 0;
		for(auto f = req->type; *f; f++) {
			if(count)
				e << ", ";
			write_array(e, object, f, level + 1);
			count++;
		}
		if(level > 0)
			e << ")";
	}
}

void write_fields(io::stream& e, const void* object, const bsreq* req, const bsreq* skip = 0) {
	auto count = 0;
	for(auto f = req; *f; f++) {
		if(skip && skip == f)
			continue;
		if(isempthy(object, req))
			continue;
		if(count > 0)
			e << " ";
		e << req->id;
		e << "(";
		write_array(e, object, req, 0);
		e << ")";
		count++;
	}
	e << "\r\n";
}

static void write_object(io::stream& e, const void* object) {
	auto pd = bsdata::findbyptr(object);
	if(!pd)
		return;
	e << "#" << pd->id << " ";
	auto skip = write_key(e, object, pd->fields);
	e << " ";
	write_fields(e, object, pd->fields, skip);
}

static void write_data(io::stream& e, bsdata* pd) {
	if(!pd)
		return;
	for(int index = 0; index < (int)pd->getcount(); index++) {
		auto object = pd->get(index);
		write_object(e, object);
	}
}

void bsdata::write(const char* url, const char** bases) {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	for(auto pname = bases; *pname; pname++)
		write_data(file, bsdata::find(*pname));
}

void bsdata::write(const char* url, const char* baseid) {
	const char* source[] = {baseid, 0};
	write(url, source);
}

void bsdata::parse(const char* value) {
	if(!value)
		return;
	bsparse parser(value);
	parser.parse();
}

void bsdata::read(const char* url) {
	auto p = loadt(url);
	if(p) {
		parse(p);
		delete p;
	}
}