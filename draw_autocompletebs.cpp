#include "bsdata.h"
#include "crt.h"
#include "draw_list.h"

using namespace draw;
using namespace draw::controls;

static const autocompletebs* sort_list;
static int compare(const void* v1, const void* v2) {
	auto p1 = (const char*)sort_list->requisit->get(sort_list->requisit->ptr(*((void**)v1)));
	auto p2 = (const char*)sort_list->requisit->get(sort_list->requisit->ptr(*((void**)v2)));
	if(!p1)
		return 1;
	if(!p2)
		return -1;
	return strcmp(p1, p2);
}

autocompletebs::autocompletebs(const bsdata* base) : autocomplete(base->fields), base(base) {
}

void autocompletebs::update() {
	assert(base);
	maximum = 0;
	auto pe = base->end();
	for(auto p = base->begin(); p < pe; p += base->size) {
		if(maximum >= sizeof(source) / sizeof(source[0]))
			break;
		if(filter) {
			if(!requisit->match(p, filter))
				continue;
		}
		source[maximum++] = p;
	}
	sort_list = this;
	qsort(source, maximum, sizeof(source[0]), compare);
}