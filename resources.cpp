#include "crt.h"
#include "io.h"
#include "resources.h"
#include "surface.h"

void* rmreserve(void* ptr, unsigned size);

static sprite* load_resource_folder(const char* url) {
	char temp[260];
	int count = 0;
	for(io::file::find fd(url); fd; fd.next()) {
		if(fd.name()[0] == '.')
			continue;
		count++;
	}
	if(!count)
		return 0;
	auto ps = (sprite*)new char[1024 * 1024 * 16];
	ps->setup(count);
	for(io::file::find fd(url); fd; fd.next()) {
		if(fd.name()[0] == '.')
			continue;
		draw::surface e(fd.fullname(temp));
		if(!e)
			continue;
		ps->store(e.ptr(0, 0), e.scanline, e.width, e.height, e.width / 2, e.height / 2,
			sprite::Auto);
	}
	ps = (sprite*)rmreserve(ps, ps->size);
	return ps;
}

sprite* gres(int id) {
	if(!id) // First resource is empthy
		return 0;
	auto& e = res::elements[id];
	if(e.notfound)
		return 0;
	if(!e.data) {
		char temp[260];
		if(e.isfolder)
			e.data = load_resource_folder(e.folder);
		else
			e.data = (sprite*)loadb(szurl(temp, e.folder, e.name, "pma"));
		if(!e.data)
			e.notfound = true;
	}
	return e.data;
}

void res::cleanup() {
	for(auto* p = res::elements; p->name; p++) {
		if(p->name) {
			delete p->data;
			p->data = 0;
			p->notfound = false;
		}
	}
}

const char* res::getname(int rid) {
	return elements[rid].name;
}