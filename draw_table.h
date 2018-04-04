#include "collections.h"
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

namespace draw {
	namespace controls {
		struct table : list {
			typedef void		(table::*renderproc)(rect rc, int index, unsigned flags, void* data, const widget & e) const;
			struct sortinfo {
				const char*		id;
				int				direction;
				operator bool() const { return id != 0; }
			};
			bsreq*				fields;
			arefc<widget>		columns;
			collection&			rows;
			struct sprite*		rowsimages;
			int					maximum_column, current_column;
			bool				no_change_count;
			bool				no_change_order;
			bool				no_change_content;
			bool				group_sort_up;
			bool				use_setting;
			bool				show_header;
			bool				show_event_rows;
			static command		commands[];
			static renderproc	renders[];
			//
			table(collection& e);
			//
			virtual unsigned	add(bool run);
			virtual unsigned	addcopy(bool run);
			widget&				addcol(unsigned flags, const char* id, const char* label = 0, int width = -1);
			virtual void*		addrow(const void* copy);
			virtual void		background(rect& rc) override;
			virtual bool		canedit(int index, const widget& e) const;
			virtual unsigned	change(bool run);
			virtual bool		changing(void* object, const char* id, unsigned flags);
			void				clear();
			virtual void		contextmenu() override;
			virtual unsigned	copy(bool run);
			virtual unsigned	exportdata(bool run);
			int					find(const char* id, const char* text, int start);
			widget*				findcol(const char* id);
			command*			getcommands() const override { return commands; }
			virtual void*		getrow(int index) { return rows.get(index); }
			virtual const char*	gettext(char* result, void* data, const char* id) const;
			void				header(rect rc);
			unsigned			importdata(bool run);
			void				inputsymbol(int id, int symbol) override;
			void				keyleft(int id) override;
			void				keyright(int id) override;
			void				renderlabel(rect rc, int index, unsigned flags, void* data, const widget & e) const;
			void				rendercheck(rect rc, int index, unsigned flags, void* data, const widget & e) const;
			void				renderfield(rect rc, int index, unsigned flags, void* data, const widget & e) const;
			void				renderimage(rect rc, int index, unsigned flags, void* data, const widget& e) const;
			void				renderno(rect rc, int index, unsigned flags, void* data, const widget & e) const;
			void				linenumber(rect rc, int index, unsigned flags, void * data, const widget & e) const;
			virtual unsigned	movedown(bool run);
			virtual unsigned	moveup(bool run);
			virtual void		prerender() override;
			virtual unsigned	remove(bool run);
			void				reposition(int width);
			virtual void		row(rect rc, int index) override;
			virtual bool		selecting(rect rc, int index, point mouse) override;
			unsigned			setting(bool run);
			void				showlabel(rect rc, const char* value, unsigned flags) const;
			void				showtext(rect rc, const char * value, unsigned flags) const;
			void				sort(const char* id, int direction = 0, int i1 = 0, int i2 = -1);
			void				sort(sortinfo* pi, int i1 = 0, int i2 = -1);
			virtual unsigned	sortas(bool run);
			virtual unsigned	sortds(bool run);
			int					totalwidth() const;
			virtual void		treemark(rect rc, int index, int level) const;
			virtual void		tuning(control** controls);
			void				validate(int direction = 1, bool editable = true);
		};
		struct tableref : table {
			arefc<void*>		source;
			tableref() : table(source) {}
			void				addelement(void* element) { source.add(element); }
			void*				getrow(int index) override { return *((void**)rows.get(index)); }
		};
	}
}