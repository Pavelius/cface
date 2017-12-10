#include "io.h"

#pragma once

namespace io {
	enum node_s : char { Number, Text, Struct, Array };
	// Application defined reader.
	// Plugin read file and post events to this class.
	struct reader {
		struct node {
			node*			parent;
			const char*		name;
			node_s			type;
			bool			skip; // set this if you want skip block
			int				index, params[12];
			//
			node(node_s type = Struct);
			node(node& parent, const char* name = "", node_s type = Struct);
			bool			operator==(const char* name) const;
			//
			int				getlevel() const;
		};
		virtual void		open(node& e) {}
		virtual void		set(node& e, const char* value) = 0;
		virtual void		close(node& e) {}
	};
	// Application create instance of this object.
	// Then write data use custom, application-defined logic.
	struct writer {
		stream&				e;
		writer(stream& e) : e(e) {}
		virtual ~writer() {}
		virtual void		open(const char* name, node_s type = Struct) {}
		virtual void		set(const char* name, int value, node_s type = Number);
		virtual void		set(const char* name, const char* value, node_s type = Text) = 0;
		virtual void		close(const char* name, node_s type = Struct) {}
	};
	struct plugin {
		const char*			name;
		const char*			fullname;
		const char*			filter;
		static plugin*		first;
		plugin*				next;
		//
		plugin();
		static plugin*		find(const char* name);
		static char*		getfilter(char* result);
		virtual const char*	read(const char* source, reader& r) = 0;
		virtual writer*		write(stream& e) = 0;
	};
	struct strategy : public reader {
		const char*			id; // Second level strategy name. If we have root level 'Setting' this can be 'Columns' or 'Windows'.
		const char*			type; // First level 'Root Name', first level xml record etc. Like 'Settings'.
		static strategy*	first;
		strategy*			next;
		//
		strategy(const char* id, const char* type);
		static strategy*	find(const char* name);
		virtual void		write(io::writer& e, void* param) = 0;
	};
	bool					read(const char* url, io::reader& e);
}