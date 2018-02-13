#include "collections.h"
#include "crt.h"
#include "io.h"

#pragma once

// Fast and simple driver for streaming binary data
struct archive {
	
	char temp[128 * 128];
	bool writemode;
	io::stream& source;

	archive(io::stream& source, bool writemode) : source(source), writemode(writemode) {}

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
	// Dynamic data collection
	template<typename T> void set(aref<T>& value) {
		set(value.count);
		for(auto& e : value)
			set(e);
	}
	// All simple types and requisites
	template<typename T> void set(T& value) {
		if(writemode)
			source.write(&value, sizeof(value));
		else
			source.read(&value, sizeof(value));
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