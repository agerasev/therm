#pragma once

#include <string>
#include <exception>

namespace gl {
class Exception : public std::exception {
private:
	std::string msg;
	
public:
	Exception(const std::string &message) {
		msg = message;
	}
	virtual ~Exception() = default;
	virtual const char *what() const noexcept {
		return msg.c_str();
	}
	std::string getMessage() const {
		return msg;
	}
};

class FileNotFoundException : public Exception {
private:
	std::string fn;
public:
	FileNotFoundException(const std::string &filename)
	    : Exception("File '" + filename + "' not found")
	{
		fn = filename;
	}
	std::string getFileName() const {
		return fn;
	}
};
}
