#include "aref.h"
#include "collection.h"
#include "draw_list.h"
#include "widget.h"

#pragma once

enum column_flags {
	ColumnSizeNormal = 0x00000000,
	ColumnSizeAuto = 0x00010000,
	ColumnSizeFixed = 0x00020000,
	ColumnSizeHide = 0x00030000,
	ColumnSizeMask = 0x00030000, // Mask for all column size flags
	ColumnSmallHilite = 0x00040000, // Column has rect like in tree
	ColumnHide = 0x00080000, // Column is invisible
	ColumnRequired = 0x00100000, // Must be filled
	ColumnReadOnly = 0x00200000, // Column not used for modify data
	ColumnHideTitleText = 0x00400000,
};

namespace draw
{
	namespace controls
	{
		struct table : public list
		{
			struct sortinfo
			{
				const char*		id;
				int				direction;
				operator bool() const { return id != 0; }
			};
			xsfield*			fields;
			aref<widget>		columns;
			collection&			rows;
			struct sprite*		rowsimages;
			int					maximum_column, current_column;
			bool				no_change_max_count;
			bool				no_change_count;
			bool				no_change_order;
			bool				no_change_content;
			bool				group_sort_up;
			bool				use_setting;
			bool				show_header;
			bool				show_event_rows;
			//
			table(collection& e);
			//
			widget&				addcol(widget::proc type, const char* id, const char* label, unsigned flags = 0, const char* link = 0, int width = -1);
			virtual void*		addrow(const void* copy) { return rows.add(copy); }
			virtual void		background(rect& rc) override;
			virtual bool		canedit(int index, const widget& e) const;
			virtual bool		changing(void* object, widget& e);
			void				clear();
			virtual void		contextmenu() override;
			int					find(const char* id, const char* text, int start);
			widget*				findcol(const char* id);
			command*			getcommands() const override;
			void				header(rect rc);
			virtual void		prerender() override;
			void				reposition(int width);
			virtual void		row(rect rc, int index) override;
			virtual bool		selecting(rect rc, int index, point mouse) override;
			bool				setting();
			void				sort(const char* id, int direction = 0, int i1 = 0, int i2 = -1);
			void				sort(sortinfo* pi, int i1 = 0, int i2 = -1);
			int					totalwidth() const;
			virtual void		treemark(rect rc, int index, int level) const;
			virtual void		tuning(control** controls);
			void				validate(int direction = 1, bool editable = true);
		};
	}
}
int tbl_check(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int tbl_date(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int tbl_image(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int tbl_linenumber(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int tbl_number(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
int tbl_reference(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);
void tbl_text(rect rc, const char* value, unsigned flags);
int tbl_text(int x, int y, int width, const char* id, unsigned flags, const char* label, int value = 0, const char* link = 0, draw::control* source = 0, int title = 0, const draw::widget* childs = 0, const char* tips = 0);