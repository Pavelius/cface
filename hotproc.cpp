#include "crt.h"
#include "hotproc.h"

hotproc::plugin* hotproc::plugin::first;

hotproc::plugin::plugin(hotproc* elements) : elements(elements) {
	seqlink(this);
}

hotproc* hotproc::plugin::find(int key) {
	for(auto pp = first; pp; pp = pp->next) {
		for(auto p = pp->elements; *p; p++) {
			if(p->key == key)
				return p;
		}
	}
	return 0;
}