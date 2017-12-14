#include "collections.h"
#include "crt.h"
#include "io.h"

#pragma once

// Fast and simple driver for streaming binary data
struct archive {
	io::stream&		source;
	bool			writemode;

	archive(io::stream& source, bool writemode) : source(source), writemode(writemode) {
	}

	// All simple types and requisites
	template<class T> void set(T& value) {
		if(writemode)
			source.write(&value, sizeof(value));
		else
			source.read(&value, sizeof(value));
	}

	// Array with fixed count
	template<typename T> void set(T value[], unsigned count) {
		for(int i = 0; i < count; i++)
			set(value[i]);
	};

	// Fixed data collection
	template<typename T, unsigned N> void set(adat<T, N>& value) {
		set(value.count);
		for(auto& e : value)
			set(e);
	}
	
	// Strings
	template<> void set<const char*>(const char*& e) {
		if(writemode) {
			unsigned len = zlen(e);
			source.write(&len, sizeof(len));
			if(len)
				source.write(e, len);
		} else {
			unsigned len;
			char temp[128 * 128];
			source.read(&len, sizeof(len));
			e = 0;
			if(len) {
				source.read(temp, len);
				temp[len] = 0;
				e = szdup(temp);
			}
		}
	}

};
