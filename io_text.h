#include "crt.h"
#include "io.h"

#pragma once

namespace io {
	struct text : public stream {
		bool				identifier(char* result, unsigned max_count);
		bool				is(const char symbol);
		bool				is(const char symbol, unsigned index);
		bool				ispair(const char* pairs);
		bool				left(const char* value);
		void				next(unsigned count);
		bool				oneof(const char* value);
		int					read(void* result, int count) override;
		int					seek(int count, int rel = SeekCur) override;
		bool				skipcr(); // Skip one \n or \r or any pair of this.  Return true if one of this case have place.
		bool				skipws(); // Skip all spaces or tabs. Return true if one of this case have place.
		int					write(const void* result, int count) override;
		text(io::stream& parent, codepages cp = CPNONE);
		operator bool() const { return false; }
	private:
		codepages			cp;
		char				cashed[64];
		unsigned			cashed_pos;
		unsigned			cashed_count;
		io::stream&			parent;
		//
		unsigned char		get();
		unsigned			getcount() const { return cashed_count - cashed_pos; }
		unsigned			getu();
		bool				makecashe(unsigned count);
		void				put(unsigned char sym) { write(&sym, sizeof(sym)); }
		void				putu(unsigned value);
		void				shift();
	};
}