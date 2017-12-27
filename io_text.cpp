#include "io_text.h"

using namespace io;

text::text(stream& parent, codepages cp_src, codepages cp_dst) : parent(parent), cp_src(cp_src), cp_dst(cp_dst) {
}

void text::autodetect(unsigned& result) {
	if(cp_src != CPNONE || result != 0xEF)
		return;
	cp_src = CP1251;
	if(get() != 0xBB)
		return;
	if(get() != 0xBF)
		return;
	cp_src = CPUTF8;
	result = get();
}

unsigned char text::get() {
	unsigned char sym;
	if(!parent.read(&sym, sizeof(sym)))
		return 0;
	return sym;
}

int	text::seek(int count, int rel) {
	return 0;
}

int	text::write(const void* result, int count) {
	return 0;
}

int text::read(void* result, int count) {
	auto pb = (char*)result;
	auto pe = pb + count;
	while(pb < pe) {
		auto value = getu();
		if(!value)
			break;
		switch(cp_dst) {
		case CPUTF8:
			if(((unsigned short)value) < 128) {
				if(pb < pe)
					*pb++ = (unsigned char)value;
			} else if(((unsigned short)value) < 2047) {
				if(pb < pe)
					*pb++ = (unsigned char)(192 + (((unsigned short)value) / 64));
				if(pb < pe)
					*pb++ = (unsigned char)(128 + (((unsigned short)value) % 64));
			} else {
				if(pb < pe)
					*pb++ = (unsigned char)(224 + (((unsigned short)value) / 4096));
				if(pb < pe)
					*pb++ = (unsigned char)(128 + ((((unsigned short)value) / 64) % 64));
				if(pb < pe)
					*pb++ = (unsigned char)(224 + (((unsigned short)value) % 64));
			}
			break;
		case CP1251:
			if(value >= 0x410 && value <= 0x44F)
				value = value - 0x410 + 0xC0;
			else switch(value) {
			case 0x406: value = 0xB2; break; // I
			case 0x407: value = 0xAF; break; // ¯
			case 0x456: value = 0xB3; break;
			case 0x457: value = 0xBF; break;
			}
			if(pb < pe)
				*pb++ = (unsigned char)value;
			break;
		case CPU16LE:
			if(pb < pe)
				*pb++ = (unsigned char)(value & 0xFF);
			if(pb < pe)
				*pb++ = (unsigned char)(((unsigned)value >> 8));
			break;
		case CPU16BE:
			if(pb < pe)
				*pb++ = (unsigned char)(((unsigned)value >> 8));
			if(pb < pe)
				*pb++ = (unsigned char)(value & 0xFF);
			break;
		default:
			if(pb < pe)
				*pb++ = (unsigned char)value;
			break;
		}
	}
	return pb - (char*)result;
}

unsigned text::getu() {
	unsigned result = get();
	autodetect(result);
	switch(cp_src) {
	case CPUTF8:
		if(result >= 192 && result <= 223)
			result = (result - 192) * 64 + (get() - 128);
		else if(result >= 224 && result <= 239) {
			auto p0 = get();
			auto p1 = get();
			result = (result - 224) * 4096 + (p0 - 128) * 64 + (p1 - 128);
		}
		return result;
	case CPU16LE:
		result |= get() << 8;
		return result;
	case CP1251:
		if(((unsigned char)result >= 0xC0))
			return result - 0xC0 + 0x410;
		else switch(result) {
		case 0xB2: return 0x406;
		case 0xAF: return 0x407;
		case 0xB3: return 0x456;
		case 0xBF: return 0x457;
		}
		return result;
	default:
		return result;
	}
}

void text::putu(unsigned value) {
	switch(cp_dst) {
	case CPUTF8:
		if(((unsigned short)value) < 128)
			put((unsigned char)value);
		else if(((unsigned short)value) < 2047) {
			put((unsigned char)(192 + (((unsigned short)value) / 64)));
			put((unsigned char)(128 + (((unsigned short)value) % 64)));
		} else {
			put((unsigned char)(224 + (((unsigned short)value) / 4096)));
			put((unsigned char)(128 + ((((unsigned short)value) / 64) % 64)));
			put((unsigned char)(224 + (((unsigned short)value) % 64)));
		}
		break;
	case CP1251:
		if(value >= 0x410 && value <= 0x44F)
			value = value - 0x410 + 0xC0;
		else switch(value) {
		case 0x406: value = 0xB2; break; // I
		case 0x407: value = 0xAF; break; // ¯
		case 0x456: value = 0xB3; break;
		case 0x457: value = 0xBF; break;
		}
		put((unsigned char)value);
		break;
	case CPU16LE:
		put((unsigned char)(value & 0xFF));
		put((unsigned char)(((unsigned)value >> 8)));
		break;
	case CPU16BE:
		put((unsigned char)(((unsigned)value >> 8)));
		put((unsigned char)(value & 0xFF));
		break;
	default:
		put((unsigned char)value);
		break;
	}
}