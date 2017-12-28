#include "crt.h"
#include "io.h"

#pragma once

namespace io {
	struct text : stream {
		char				get();
		char				getnr();
		bool				identifier(char* result, unsigned max_count);
		bool				is(const char symbol);
		bool				is(const char symbol, unsigned index);
		bool				is(const char* one_of_symbols);
		bool				islinefeed();
		bool				ispair(const char* pairs);
		void				next(unsigned count);
		bool				next(const char* value);
		bool				skipcr(); // Skip one \n or \r or any pair of this.  Return true if one of this case have place.
		bool				skipws(); // Skip all spaces or tabs. Return true if one of this case have place.
		text(io::stream& parent, codepages cp = CPNONE);
		operator bool() const { return false; }
	private:
		codepages			cp;
		char				cashed[64];
		unsigned			cashed_pos;
		unsigned			cashed_count;
		io::stream&			parent;
		bool				end_of_file;
		//
		unsigned char		getone();
		unsigned			getcount() const { return cashed_count - cashed_pos; }
		unsigned			getu();
		bool				makecashe(unsigned count);
		void				put(unsigned char sym) { write(&sym, sizeof(sym)); }
		void				putu(unsigned value);
		int					read(void* result, int count) override;
		int					seek(int count, int rel = SeekCur) override;
		void				shift();
		int					write(const void* result, int count) override;
	};
}