#include "crt.h"
#include "io.h"

#pragma once

namespace io {
	struct text : stream {
		int					read(void* result, int count) override;
		int					seek(int count, int rel = SeekCur) override;
		int					write(const void* result, int count) override;
		text(stream& parent, codepages cp_src = CPNONE, codepages cp_dst = CP1251);
	private:
		codepages			cp_src;
		codepages			cp_dst;
		stream&				parent;
		void				autodetect(unsigned& result);
		unsigned char		get();
		unsigned			getu();
		void				put(unsigned char sym) { write(&sym, sizeof(sym)); }
		void				putu(unsigned sym);
	};
	struct sequence : public stream {
		unsigned char		get();
		bool				left(const char* value);
		int					read(void* result, int count) override;
		int					seek(int count, int rel = SeekCur) override;
		int					write(const void* result, int count) override;
		sequence(io::stream& parent, codepages cp_src = CPNONE);
		operator bool() const { return false; }
	private:
		codepages			cp_src;
		codepages			cp_dst;
		char				cashed[64];
		unsigned			cashed_pos;
		unsigned			cashed_count;
		io::stream&			parent;
		//
		unsigned char		getb();
		unsigned			getcount() const { return cashed_count - cashed_pos; }
		unsigned			getu();
		bool				makecashe(unsigned count);
		void				shift();
	};
}