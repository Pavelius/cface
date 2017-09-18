#include "crt.h"
#include "draw.h"
#include "widget.h"

int draw::sheetline(rect rc, bool background)
{
	if(background)
		draw::gradv(rc, colors::border.lighten(), colors::border.darken());
	draw::rectf({rc.x1, rc.y2 - 2, rc.x2, rc.y2}, colors::active);
	return rc.height();
}

static bool sheet(rect& rc, rect& rct, const char* string, areas* area_result, bool checked, bool right_side, int w2)
{
	bool result = false;
	int width = draw::textw(string) + w2;
	if(right_side)
		rct.set(rc.x2 - width - 8, rc.y1, rc.x2, rc.y2);
	else
		rct.set(rc.x1, rc.y1, rc.x1 + width + 8, rc.y2);
	auto a = draw::area(rct);
	if(area_result)
		*area_result = a;
	if(checked)
	{
		draw::gradv({rct.x1 + 1, rct.y1 + 1, rct.x2, rct.y2 - 2},
			colors::tabs::back, colors::active);
		draw::rectb({rct.x1 + 1, rct.y1 + 1, rct.x2, rct.y2 - 1},
			colors::active);
	}
	draw::text({rct.x1, rct.y1, rct.x2 - w2, rct.y2}, string, AlignCenterCenter);
	if(a != AreaNormal)
	{
		if(a == AreaHilitedPressed)
		{
			if(hot::key == MouseLeft)
				result = true;
		}
		else if(a == AreaHilited)
		{
			draw::rectf({rct.x1 + 1, rct.y1 + 1, rct.x2, rct.y2 - 2},
				colors::tabs::back, 64);
		}
	}
	if(right_side)
		rc.x2 -= rct.width();
	else
		rc.x1 += rct.width();
	return result;
}

int draw::tabs(rect rc, bool show_close, bool right_side, void** data, int start, int count, int current, int* hilite,
	draw::proctext gtext, draw::proctext gstate, rect position)
{
	draw::state push;
	rect rco = rc + position;
	draw::setclip(rc);
	char temp[260];
	auto result = 0;
	// Получим выравнивание
	int ox1 = rc.x1;
	for(int i = start; i < count; i++)
	{
		if(rc.x1 >= rc.x2 || rc.x2 <= rc.x1)
			break;
		auto object = data[i];
		auto s = gtext(temp, object);
		if(i == current)
			fore = colors::tabs::text;
		else
			fore = colors::text;
		areas a; rect element;
		if(sheet(rc, element, s, &a, (i == current), right_side, (show_close ? 16 : 0)))
			result = 1;
		if(a == AreaHilited || a == AreaHilitedPressed)
		{
			if(hilite)
				*hilite = i;
			//if(gstate)
			//	statusbar(gstate(temp, data[i]));
		}
		if((a == AreaHilited || a == AreaHilitedPressed || (i == current)) && show_close)
		{
			const int dy = 12;
			rect rcx;
			rcx.y1 = element.y1 + (element.height() - dy) / 2;
			rcx.y2 = rcx.y1 + dy;
			rcx.x1 = element.x2 - dy - 4;
			rcx.x2 = rcx.x1 + rcx.height();
			if(areb(rcx))
			{
				if(buttonh(rcx, false, false, false, true, 0))
					result = 2;
				tooltips("Закрыть");
			}
			line(rcx.x1 + 4, rcx.y1 + 4, rcx.x2 - 4, rcx.y2 - 4, fore);
			line(rcx.x2 - 4, rcx.y1 + 4, rcx.x1 + 4, rcx.y2 - 4, fore);
		}
	}
	//return rc.x1 - ox1;
	return result;
}

static char* get_text(char* result, void* object)
{
	if(((draw::widget*)object)->label)
		zcpy(result, ((draw::widget*)object)->label, 259);
	return result;
}

int wdt_tabs(int x, int y, int width, draw::element& e)
{
	if(!e.childs)
		return 0;
	auto y0 = y;
	const int tab_height = 24 + 4;
	y += 1;
	draw::sheetline({x, y0, x + width, y + tab_height});
	const draw::widget* data[32];
	auto ps = data;
	auto pe = data + sizeof(data)/sizeof(data[0]);
	for(auto p = e.childs; *p; p++)
	{
		if(ps < pe)
			*ps++ = p;
	}
	auto count = ps - data;
	int current = e.data.get();
	rect rc = {x, y, x + width, y + tab_height};
	int tabs_hilite;
	if(draw::tabs(rc, false, false, (void**)data, 0, count, current, &tabs_hilite, get_text))
	{
		if(tabs_hilite!=-1)
			execute(InputSetValue, tabs_hilite, e);
	}
	y += tab_height + metrics::padding;
	draw::element e1(e.childs[current], &e);
	return e1.type(x, y, width, e1) + (y - y0);
}