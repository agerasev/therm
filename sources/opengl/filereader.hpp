#pragma once

#include <cstdio>
#include <string>
#include <memory>

#include "exception.hpp"

namespace gl {
class FileReader {
private:
	FILE *file = nullptr;
	char *data = nullptr;
	long size = 0;
	
public:
	FileReader(const std::string &filename) throw(FileNotFoundException) {
		file = fopen(filename.c_str(), "r");
		if(file == nullptr)
			throw FileNotFoundException(filename);
		
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fseek(file, 0, SEEK_SET);
		
		data = new char[size + 1];
		long chars_read = fread(data, 1, size, file);
		data[chars_read] = '\0';
	}
	~FileReader() {
		if(data != nullptr)
			delete[] data;
		if(file != nullptr)
			fclose(file);
	}
	char *getData() {
		return data;
	}
	long getSize() const {
		return size;
	}
};
}
