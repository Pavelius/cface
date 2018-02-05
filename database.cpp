#include "bsdata.h"
#include "crt.h"
#include "database.h"

namespace database {

adat<object, 256 * 256 * 4> objects;
object						root;

bsreq object_type[] = {
	BSREQ(object, name, text_type),
	BSREQ(object, parent, object_type),
};
bsdata object_manager("object", objects.data, objects.count, sizeof(objects.data[0]), sizeof(objects.data) / sizeof(objects.data[0]), object_type);

object* object::find(const char* name) {
	for(auto& e : objects) {
		if(e.name == name)
			return &e;
	}
	return 0;
}

void object::clear() {
	memset(this, 0, sizeof(*this));
}

object& object::add(const char* name) {
	name = szdup(name);
	auto p = find(name);
	if(!p)
		p = objects.add();
	if(!p)
		p = objects.data;
	p->parent = this;
	p->name = szdup(name);
	return *p;
}

}