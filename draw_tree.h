#include "amem.h"
#include "draw_table.h"

#pragma once

enum treeflags {
	TIGroup = 1, TICheck = 2,
};

namespace draw {
	namespace controls {
		struct tree : table, amem {
			// Basic child element of every tree line
			struct element {
				unsigned char	level;
				unsigned char	flags;
				unsigned char	type;
				unsigned char	image;
				unsigned		param;
			};
			int				level;
			int				index;
			bool			sort_rows_by_name;
			bool			element_param_have_row;
			//
			tree(unsigned size = sizeof(element));
			//
			void			addrow(element& e); // Add new row when use expanding()
			void			addrow(unsigned param, unsigned char flags, unsigned char type = 0, unsigned char image = 0); // Add new row when use expanding()
			void			addrow(void* object); // Add new row outside expanding()
			void			clear();
			void			collapse(int i);
			virtual void	expand(int row, int level) final;
			virtual void	expanding() {} // Expanding 'row' with 'level' or 0 for root
			int				findbyparam(int param) const;
			virtual int		getimageindex(int index) const override;
			virtual int		getlevel(int row) const override;
			int				getparam(int row = -1) const;
			int				getparamindex(int value) const;
			void*			getrow(int value) override;
			int				gettype(int row = -1) const;
			bool			haselement(int param) const;
			virtual bool	isgroup(int row) const override;
			void			open(int max_level);
			unsigned		sortas(bool run) override;
			unsigned		sortds(bool run) override;
		};
	}
}