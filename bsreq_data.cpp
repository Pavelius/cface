#include "crt.h"
#include "bsdata.h"

static const bsreq* getkeyreq(const bsreq* type) {
	for(auto p = type; *p; p++) {
		if(p->type == text_type)
			return p;
	}
	return 0;
}

const char* bsreq::getdata(char* result, const char* id, const void* object, bool tobuffer) const {
	result[0] = 0;
	auto requisit = find(id);
	if(!requisit)
		return 0;
	if(requisit->type == text_type) {
		const char* value = 0;
		if(requisit->size == sizeof(char) && requisit->count > 1)
			value = (const char*)requisit->ptr(object);
		else
			value = (const char*)requisit->get(requisit->ptr(object));
		if(value) {
			if(!tobuffer)
				return value;
			zcpy(result, value);
		}
	} else if(requisit->type == number_type) {
		auto value = requisit->get(requisit->ptr(object));
		sznum(result, value);
	} else {
		if(requisit->reference)
			object = (void*)requisit->get(requisit->ptr((void*)object));
		else if(requisit->isenum) {
			auto xs = bsdata::find(requisit->type);
			if(!xs)
				return result;
			auto index = requisit->get(requisit->ptr(object));
			object = xs->get(index);
		}
		if(!object)
			return result;
		requisit = getkeyreq(requisit->type);
		if(!requisit)
			return result;
		auto value = (const char*)requisit->get(requisit->ptr(object));
		if(value) {
			if(!tobuffer)
				return value;
			zcpy(result, value);
		}
	}
	return result;
}

void bsreq::setdata(const char* result, const char* id, void* object) const {
	auto requisit = find(id);
	if(!requisit)
		return;
	if(requisit->type == text_type) {
		if(result[0] == 0)
			result = 0;
		else
			result = szdup(result);
		requisit->set(requisit->ptr(object), (int)result);
	} else if(requisit->type == number_type)
		requisit->set(requisit->ptr(object), sz2num(result));
	else if(result[0] == 0)
		requisit->set(requisit->ptr(object), 0);
	else {
		auto b = bsdata::find(requisit->type);
		if(b) {
			auto f = getkeyreq(b->fields);
			if(f) {
				if(requisit->reference)
					requisit->set(requisit->ptr(object), (int)b->find(f, result));
				else {
					auto i = b->indexof(b->find(f, result));
					if(i == -1)
						i = 0;
					requisit->set(requisit->ptr(object), i);
				}
			}
		}
	}
}