#include "bsdata.h"
#include "crt.h"
#include "draw.h"
#include "draw_textedit.h"
#include "draw_table.h"
#include "widget.h"

using namespace draw;

static void*			hot_object;
static const bsreq*		hot_type;

static const char* get_text(char* result, void* object) {
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
	hot_type->set(hot_type->ptr(hot_object), p->value);
}

static void callback_check() {
	auto p = (widget*)hot::param;
	auto po = (void*)hot_type->ptr(hot_object);
	auto pv = hot_type->get(po);
	if(p->value) {
		if((pv & p->value) == 0)
			pv |= p->value;
		else
			pv &= ~p->value;
	} else if(pv)
		hot_type->set(po, 0);
	else
		hot_type->set(po, 1);
}

static void callback_edit() {
	auto p = (widget*)hot::param;
	auto current_object = hot_object;
	auto current_type = hot_type;
	char temp[4196]; temp[0] = 0;
	if(!current_type->getdata(temp, p->id, hot_object, true)) {
		hot::key = 0;
		return;
	}
	controls::textedit te(temp, sizeof(temp), true);
	te.align = p->flags;
	auto b = bsdata::find(hot_type->type);
	bool result = false;
	bool need_dropdown = (hot_type->type != text_type && hot_type->type != number_type)
		&& (hot_type->reference || hot_type->isenum)
		&& b;
	if(need_dropdown) {
		controls::autocompletebs aclist(b);
		te.records = &aclist;
		aclist.hilite_rows = true;
		result = te.editing(hot::element);
	} else
		result = te.editing(hot::element);
	if(result)
		hot_type->setdata(temp, p->id, hot_object);
	// Some keys must be handled by this control
	switch(hot::key) {
	case KeyDown:
	case KeyUp:
		draw::execute(hot::key, 0);
		break;
	}
}

static void callback_up() {
	auto p = (widget*)hot::param;
	auto po = (void*)hot_type->ptr(hot_object);
	hot_type->set(po, hot_type->get(po) - 1);
}

static void callback_down() {
	auto p = (widget*)hot::param;
	auto po = (void*)hot_type->ptr(hot_object);
	hot_type->set(po, hot_type->get(po) + 1);
}

struct dlgform {

	bsval*			variables;
	rect			client;

	unsigned getflags(const widget& e) const {
		return e.flags;
	}

	bsval getinfo(const char* id) const {
		for(auto po = variables; *po; po++) {
			auto f = po->type->find(id);
			if(!f)
				continue;
			return {f, po->data};
		}
		return {0, 0};
	}

	bsval getinfo(const char* id, bsreq* type) const {
		for(auto po = variables; *po; po++) {
			auto f = po->type->find(id, type);
			if(!f)
				continue;
			return {f, po->data};
		}
		return {0, 0};
	}

	void setdata(const widget& e, int value, bool instant = false) {
		auto po = getinfo(e.id);
		if(!po)
			return;
		if(instant)
			po.set(value);
		else {
			hot_object = (void*)po.type->ptr(po.data);
			hot_type = po.type;
			execute(callback_setdata);
			hot::param = value;
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
		auto po = getinfo(e.id);
		if(!po)
			return 0;
		char temp[260];
		auto p = po.type->getdata(temp, e.id, po.data, false);
		if(!p)
			return 0;
		auto flags = getflags(e);
		if(po.type->type == number_type)
			return draw::field(x, y, width, (int)&e, getflags(e), p, e.tips, e.label, e.title,
				callback_edit, 0, 0, callback_up, callback_down, 0, setparam, &po);
		else if(po.type->type != text_type)
			return draw::fieldl(x, y, width, (int)&e, getflags(e), p, e.tips, e.label, e.title,
				0, 0, setparam, &po);
		else
			return draw::field(x, y, width, (int)&e, getflags(e), p, e.tips, e.label, e.title,
				callback_edit, 0, 0, 0, 0, 0, setparam, &po);
	}

	int tabs(int x, int y, int width, const widget& e) {
		if(!e.childs)
			return 0;
		auto po = getinfo(e.id);
		if(!po)
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
		auto current = po.get();
		rect rc = {x, y, x + width, y + tab_height};
		int tabs_hilite = -1;
		if(draw::tabs(rc, false, false, (void**)data, 0, count, current, &tabs_hilite, get_text, 0, {0, 0, 0, 0})) {
			if(tabs_hilite != -1)
				setdata(e, tabs_hilite);
		}
		y += tab_height + metrics::padding;
		return element(x, y, width, e.childs[current]);
	}

	int custom(int x, int y, int width, const widget& e) {
		auto po = getinfo(e.id, control_type);
		if(!po)
			return 0;
		auto pc = (draw::control*)po.type->ptr(po.data);
		auto height = client.height() - y - metrics::padding*2;
		pc->viewf({x, y, x + width, y + height}, true);
		return height + metrics::padding;
	}

	int decoration(int x, int y, int width, const widget& e) {
		draw::state push;
		auto flags = getflags(e);
		setposition(x, y, width);
		decortext(flags);
		draw::link[0] = 0;
		auto height = draw::textf(x, y, width, e.label) + metrics::padding * 2;
		if(draw::link[0])
			tooltips(link);
		return height;
	}

	int radio(int x, int y, int width, const widget& e) {
		if(!e.label || !e.label[0] || !e.id || !e.id[0])
			return 0;
		auto po = getinfo(e.id);
		if(!po)
			return 0;
		auto flags = getflags(e);
		if(po.get() == e.value)
			flags |= Checked;
		return draw::radio(x, y, width, (int)&e, flags, e.label, e.tips, callback_radio, setparam, &po);
	}

	int check(int x, int y, int width, const widget& e) {
		if(!e.label || !e.label[0] || !e.id || !e.id[0])
			return 0;
		auto po = getinfo(e.id);
		if(!po)
			return 0;
		auto flags = getflags(e);
		if(po.get() == e.value)
			flags |= Checked;
		return draw::checkbox(x, y, width, (int)&e, flags, e.label, e.tips, callback_check, setparam, &po);
	}

	int button(int x, int y, int width, const widget& e) {
		auto po = getinfo(e.id);
		// Hot keys must work, if we want this
		if(e.value && hot::key == e.value)
			doevent((int)&e, e.callback, setparam, &po);
		return draw::button(x, y, width, (int)&e, getflags(e), e.label, e.tips,
			e.callback, setparam, &po);
	}

	int renderno(int x, int y, int width, const widget& e) {
		return 0;
	}

	int element(int x, int y, int width, const widget& e) {
		typedef int (dlgform::*callback)(int, int, int, const widget&);
		static callback methods[] = {
			&dlgform::renderno,
			&dlgform::decoration, &dlgform::group, &dlgform::tabs, &dlgform::custom,
			&dlgform::button, &dlgform::field, &dlgform::check, &dlgform::radio,
			&dlgform::renderno, &dlgform::renderno
		};
		return (this->*methods[e.gettype()])(x, y, width, e);
	}

	dlgform(bsval* variables) : variables(variables) {
	}

};

static void setparam(void* param) {
	hot_object = ((bsval*)param)->data;
	hot_type = ((bsval*)param)->type;
}

int	draw::render(int x, int y, int width, int height, const widget* p, bsval* variables) {
	dlgform e(variables);
	e.client = {x, y, x + width, y + height};
	if(p->width)
		return e.horizontal(x, y, width, p);
	return e.vertical(x, y, width, p);
}

int draw::open(const char* title, int width, int height, const widget* widgets, bsval* variables, bool(*validate)(bsval* variables)) {
	struct context : control {
		void* object;
		bsval* variables;
		const widget* widgets;
		bool(*validate)(bsval* variables);
		void nonclient(rect rc) override {
			auto y0 = rc.y1;
			rc.y1 += draw::render(rc.x1, rc.y1, rc.width(), rc.height(), widgets, variables);
			if(validate) {
				bool result = validate(variables);
				button(rc.x2 - 100, rc.y1, 100, (int)"OK", result ? 0 : Disabled, "OK", 0, buttonok);
				rc.x2 -= 100 - metrics::padding;
				button(rc.x2 - 100, rc.y1, 100, (int)"Cancel", result ? 0 : Disabled, "Отмена", 0, buttoncancel);
			}
		}
	} current;
	current.widgets = widgets;
	current.variables = variables;
	current.validate = validate;
	current.show_background = false;
	current.show_border = false;
	return current.open(title, WFResize, width, height, false);
}