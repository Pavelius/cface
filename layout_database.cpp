#include "array.h"
#include "base.h"
#include "base_engine.h"
#include "crt.h"
#include "dialog.h"
#include "io.h"
#include "vars.h"
#include "zarray.h"

static int last_parent;

static struct database_grid : public widgets::groups
{

	database_grid()
	{
		dock = DockLeft;
		root = Root;
		columns.add(Image, Image, FO(element,param), 20, ColumnSizeHide);
		columns.add(Name, Text, Name, 200, ColumnSizeAuto|ColumnSmallHilite);
		no_change_content = true;
		no_change_order = true;
		show_header = false;
	}

} navigator_instance;
static plugin<widgets::control> plugin_instance(navigator_instance);

//static struct database_perespective : public layout, public widgets::groups
//{
//
//	database_grid	list;
//	unsigned		list_base;
//	int				header_width;
//	bsobject*	tabs[128];
//	int				tabs_count, tabs_current, tabs_hilite;
//
//	static const char* tabs_text(char* temp, void* p)
//	{
//		bsobject* f = (bsobject*)p;
//		if(f->original==Path)
//			return szt("Workspace","Рабочий стол");
//		const char* pn = f->gets(Name);
//		if(!pn || pn[0]==0)
//			zcpy(temp, bsgets(f->get(Parent), Name));
//		else
//			zcpy(temp, pn);
//		if(f->ismodified())
//			zcat(temp, "*");
//		return temp;
//	}
//
//	static const char* tabs_state(char* temp, void* p)
//	{
//		return "";
//	}
//
//	void update_tabs()
//	{
//		static unsigned last_counter;
//		if(last_counter==bsforms_counter)
//			return;
//		last_counter = bsforms_counter;
//		bsobject** e1 = tabs;
//		for(int i=0;i<tabs_count; i++)
//		{
//			if(tabs[i]->operator bool())
//				*e1++ = tabs[i];
//		}
//		for(auto& e : bsforms)
//		{
//			if(e.operator bool() && zfind(tabs, &e)==-1)
//				*e1++ = &e;
//		}
//		*e1 = 0;
//		tabs_count = e1 - tabs;
//		if(tabs_current>=tabs_count)
//			tabs_current = tabs_count-1;
//	}
//
//	void save_current_columns()
//	{
//		if(list_base!=0xFFFFFFFF)
//		{
//			bstable& source = bstables[list_base];
//			vars::save(source.identifier, list.columns);
//		}
//	}
//
//	void update_columns()
//	{
//		if(list_base==GB(list.source))
//			return;
//		save_current_columns();
//		list_base = GB(list.source);
//		list.columns.clear();
//		bstable& source = bstables[GB(list.source)];
//		for(int i = 0; source.columns[i]; i++)
//		{
//			int id = source.columns[i];
//			if(ldb_create_columns)
//			{
//				if(!ldb_create_columns(list.source, id))
//					continue;
//			}
//			int it = bsget(id, Type);
//			switch(it)
//			{
//			case Image:
//				list.columns.add(id, it, id, 20, ColumnSizeFixed|ColumnHide);
//				break;
//			default:
//				list.columns.add(id, it, id, 200, ColumnSizeNormal);
//				break;
//			}
//		}
//		vars::load(source.identifier, list.columns);
//	}
//
//	void update_source()
//	{
//		int id = bsfirstby(groups::geti(Value), Root, Parent);
//		int first = bsfirstrec(id);
//		if(first==list.source && last_parent==groups::geti(Value))
//			return;
//		last_parent = groups::geti(Value);
//		list.source = first;
//		list.tree::clear();
//		list.selectors.clear();
//		list.selectors.add(Parent, InHierarhy, last_parent);
//		list.expand(0, 0);
//	}
//
//	void activate(bsobject* e)
//	{
//		if(!e)
//			return;
//		update_tabs();
//		for(auto& a : tabs)
//		{
//			if(a==e)
//			{
//				tabs_current = &a-tabs;
//				return;
//			}
//		}
//	}
//
//	int execute(int id, bool run)
//	{
//		switch(id)
//		{
//		case Save:
//			if(tabs_current==-1 || tabs_current==0)
//				return Disabled;
//			if(!tabs[tabs_current]->ismodified())
//				return Disabled;
//			if(run)
//			{
//				if(tabs[tabs_current])
//				{
//					tabs[tabs_current]->write();
//					last_parent = -1;
//				}
//			}
//			break;
//		case Add:
//			if(run)
//				activate(bsopen(groups::geti(Value)));
//			break;
//		case AddCopy:
//			if(run)
//				activate(bsopen(groups::geti(Value), list.geti(Key)));
//			break;
//		default:
//			return widgets::groups::execute(id, run);
//		}
//		return Executed;
//	}
//
//	const char* render(widget& dc, const char* current) override
//	{
//		if(!groups::count)
//			groups::expand(0, 0);
//		update_source();
//		vars::user.get("Forms.LayoutDatabase.HeaderWidth", header_width);
//		while(true)
//		{
//			rect rt;
//			int x = 0;
//			int y = layout::toolbar(dc, current, rt);
//			int y2 = dc.height;
//			int x2 = dc.width;
//			y2 -= dc.statusbar();
//			dc.rectf(x, y, x2, y2, colors::form);
//			x += metrics::padding;
//			y += metrics::padding;
//			x2 -= metrics::padding;
//			y2 -= metrics::padding;
//			update_tabs();
//			rt.x1 += groups::toolbar(dc, rt, 1, 0);
//			rt.x1 += dc.tabs({rt.x1, rt.y1+5, rt.x2, rt.y2+3}, 500, HideBackground|HideClose, (void**)tabs,
//				0, 1, tabs_current, &tabs_hilite, tabs_text);
//			rt.x1 += dc.tabs({rt.x1, rt.y1+5, rt.x2, rt.y2+3}, 500, HideBackground, (void**)tabs,
//				1, tabs_count, tabs_current, &tabs_hilite, tabs_text);
//			// Workspace
//			if(tabs_current==0)
//			{
//				dc.splitv(x, y, header_width, y2-y, 4, 6, 64, 282);
//				dc.view(*this, x, y, header_width, y2-y, 1, HideActiveBorder|HiliteEventRows|HideToolbar);
//				x += header_width + 6;
//				update_source();
//				update_columns();
//				dc.view(list, x, y, x2-x, y2-y, 2, HideActiveBorder|HiliteEventRows);
//			}
//			else
//			{
//				if(ldb_paint_form)
//					ldb_paint_form(dc, *tabs[tabs_current], {x,y,x2-x,y2-y});
//			}
//			int id = dc.input();
//			switch(id)
//			{
//			case 0:
//				save_current_columns();
//				return 0;
//			case 1:
//				break;
//			case 500:
//				tabs_current = hot::param;
//				break;
//			case 501:
//				if(tabs_hilite==-1 || tabs_hilite==0)
//					break;
//				if(tabs[tabs_hilite])
//				{
//					tabs[tabs_hilite]->clear();
//					bsforms_counter++;
//				}
//				break;
//			case 502:
//				activate(bsopen(list.geti(Key)));
//				break;
//			case MoveLeft:
//			case MoveRight:
//				break;
//			case Perespective:
//				return layout::gname(hot::param);
//			default:
//				if(ldb_input_form && ldb_input_form(dc, *tabs[tabs_current], id))
//					break;
//				if(dc.active)
//					dc.active->execute(id, true);
//				break;
//			}
//		}
//	}
//
//	const char* leaving(const char* name) override
//	{
//		save_current_columns();
//		vars::user.set("Forms.LayoutDatabase.HeaderWidth", header_width);
//		return name;
//	}
//
//	database_perespective() : layout("Database")
//	{
//		static int header_tools[] = {Add, AddCopy, Save, Delete, 0};
//		memset(tabs, 0, sizeof(tabs));
//		header_width = 200;
//		show_header = false;
//		groups::commands = header_tools;
//		groups::root = Root;
//		groups::columns.add(Image, Image, FO(element,param), 20, ColumnSizeHide);
//		groups::columns.add(Name, Text, Name, 200, ColumnSizeAuto|ColumnSmallHilite);
//		groups::no_change_content = true;
//		groups::no_change_order = true;
//		list_base = -1;
//		list.source = 0;
//		list.no_change_content = true;
//		tabs_count = 0;
//		tabs_current = 0;
//		tabs_hilite = -1;
//	}
//
//} instance;
//static plugin<layout> instance_plugin(&instance, sizeof(instance));