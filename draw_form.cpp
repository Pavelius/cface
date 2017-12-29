#include "bsreq.h"
#include "crt.h"
#include "draw.h"
#include "widget.h"

using namespace draw;

static void*		hot_object;
static const bsreq*	hot_type;

static char* get_text(char* result, void* object) {
	if(((widget*)object)->label)
		zcpy(result, ((widget*)object)->label, 259);
	return result;
}

static void setparam(void* param);

static void callback_setdata() {
	hot_type->set(hot_object, hot::param);
}

static void callback_radio() {
	auto p = (widget*)hot::param;
	auto f = hot_type->find(p->id);
	if(f)
		f->set(f->ptr(hot_object), p->value);
}

static void callback_check() {
	auto p = (widget*)hot::param;
	auto f = hot_type->find(p->id);
	if(f) {
		auto po = (void*)f->ptr(hot_object);
		auto pv = f->get(po);
		if(p->value) {
			if((pv & p->value) == 0)
				pv |= p->value;
			else
				pv &= ~p->value;
		} else if(pv)
			f->set(po, 0);
		else
			f->set(po, 1);
	}
}

struct dlgform {

	void*			object;
	const bsreq*	type;

	unsigned getflags(const widget& e) const {
		unsigned result = e.flags;
		if(getfocus() == (int)&e)
			result |= Focused;
		return result;
	}

	int getdata(const widget& e) const {
		auto f = type->find(e.id);
		if(!f)
			return 0;
		return f->get(f->ptr(object));
	}

	void setdata(const widget& e, int value, bool instant = false) const {
		auto f = type->find(e.id);
		if(!f)
			return;
		if(instant)
			f->set(f->ptr(object), value);
		else {
			hot_object = (void*)f->ptr(object);
			hot_type = f;
			hot::param = value;
			execute(callback_setdata);
		}
	}

	int vertical(int x, int y, int width, const widget* p) {
		if(!p)
			return 0;
		int y0 = y;
		while(*p) {
			y += element(x, y, width, *p);
			p++;
		}
		return y - y0;
	}

	int horizontal(int x, int y, int width, const widget* p) {
		if(!p)
			return 0;
		int mh = 0;
		int n = 0;
		while(*p) {
			auto w = width*p->width / 12;
			auto x1 = x + width*n / 12;
			auto h = element(x1, y, w, *p);
			if(h > mh)
				mh = h;
			n += p->width;
			p++;
		}
		return mh;
	}

	int group(int x, int y, int width, const widget& e) {
		if(!e.childs)
			return 0;
		int y0 = y;
		if(e.label) {
			setposition(x, y, width); // Первая рамка (может надо двойную ?)
			int x1 = x, y1 = y, w1 = width;
			setposition(x, y, width); // Отступ от рамки
			draw::state push;
			draw::font = metrics::font;
			if(e.label)
				y += texth() + metrics::padding * 2;
			auto w = 0;
			if(e.childs[0].width)
				w = horizontal(x, y, width, e.childs);
			else
				w = vertical(x, y, width, e.childs);
			if(w == 0)
				return 0;
			y += w;
			color c1 = colors::border.mix(colors::window, 128);
			color c2 = c1.darken();
			gradv({x1, y1, x1 + w1, y1 + texth() + metrics::padding * 2}, c1, c2);
			fore = colors::text.mix(c1, 96);
			text(x1 + (w1 - textw(e.label)) / 2, y1 + metrics::padding, e.label);
			rectb({x1, y1, x1 + w1, y}, colors::border);
			y += metrics::padding * 2;
		} else {
			if(e.childs[0].width)
				y += horizontal(x, y, width, e.childs);
			else
				y += vertical(x, y, width, e.childs);
		}
		return y - y0;
	}

	int field(int x, int y, int width, const widget& e) {
		if(!e.id || !e.id[0])
			return 0;
		char temp[260];
		auto f = this->type->find(e.id);
		auto p = f->getdata(temp, e.id, object, false);
		if(!p)
			return 0;
		auto flags = getflags(e);
		if(f == number_type)
			return draw::field(x, y, width, (int)&e, getflags(e), p, e.tips, e.label, e.title);
		else if(f != text_type)
			return draw::field(x, y, width, (int)&e, getflags(e), p, e.tips, e.label, e.title);
		else
			return draw::field(x, y, width, (int)&e, getflags(e), p, e.tips, e.label, e.title);
	}

	int tabs(int x, int y, int width, const widget& e) {
		if(!e.childs)
			return 0;
		auto y0 = y;
		const int tab_height = 24 + 4;
		y += 1;
		sheetline({x, y0, x + width, y + tab_height});
		const widget* data[32];
		auto ps = data;
		auto pe = data + sizeof(data) / sizeof(data[0]);
		for(auto p = e.childs; *p; p++) {
			if(ps < pe)
				*ps++ = p;
		}
		auto count = ps - data;
		auto current = getdata(e);
		rect rc = {x, y, x + width, y + tab_height};
		int tabs_hilite = -1;
		if(draw::tabs(rc, false, false, (void**)data, 0, count, current, &tabs_hilite, get_text, 0, {0, 0, 0, 0})) {
			if(tabs_hilite != -1)
				setdata(e, tabs_hilite);
		}
		y += tab_height + metrics::padding;
		return element(x, y, width, e.childs[current]);
	}

	int decoration(int x, int y, int width, const widget& e) {
		draw::state push;
		auto flags = getflags(e);
		setposition(x, y, width);
		decortext(flags);
		return draw::textf(x, y, width, e.label) + metrics::padding * 2;
	}

	int radio(int x, int y, int width, const widget& e) {
		if(!e.label || !e.label[0] || !e.id || !e.id[0])
			return 0;
		auto flags = getflags(e);
		if(getdata(e) == e.value)
			flags |= Checked;
		return draw::radio(x, y, width, (int)&e, flags, e.label, e.tips, callback_radio, setparam, this);
	}

	int check(int x, int y, int width, const widget& e) {
		if(!e.label || !e.label[0] || !e.id || !e.id[0])
			return 0;
		auto flags = getflags(e);
		if(getdata(e))
			flags |= Checked;
		return draw::checkbox(x, y, width, (int)&e, flags, e.label, e.tips, callback_check, setparam, this);
	}

	int button(int x, int y, int width, const widget& e) {
		return draw::button(x, y, width, (int)&e, getflags(e), e.label, e.tips, 0);
	}

	int renderno(int x, int y, int width, const widget& e) {
		return 0;
	}

	int element(int x, int y, int width, const widget& e) {
		typedef int (dlgform::*callback)(int, int, int, const widget&);
		static callback methods[] = {
			&dlgform::renderno,
			&dlgform::decoration, &dlgform::group, &dlgform::tabs,
			&dlgform::renderno, &dlgform::button, &dlgform::field, &dlgform::check, &dlgform::radio,
			&dlgform::renderno, &dlgform::renderno
		};
		return (this->*methods[e.gettype()])(x, y, width, e);
	}

	dlgform(void* object, const bsreq* type) : object(object), type(type) {
	}

};

static void setparam(void* param) {
	hot_object = ((dlgform*)param)->object;
	hot_type = ((dlgform*)param)->type;
}

int	draw::render(int x, int y, int width, const widget* p, void* object, bsreq* type) {
	dlgform e(object, type);
	return e.vertical(x, y, width, p);
}