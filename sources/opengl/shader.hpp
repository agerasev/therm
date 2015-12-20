#pragma once

#include <cstdio>
#include <list>
#include <string>
#include <regex>

#include <GL/glew.h>

#include "filereader.hpp"

namespace gl {
class Shader {
public:
	enum Type {
		VERTEX,
		FRAGMENT
	};
	struct Variable {
		std::string type;
		std::string name;
	};
	
private:
	GLuint _id = 0;
	std::string _name = "";
	std::list<Variable> _attribs, _uniforms;
	
public:
	Shader(Shader::Type type)
	{
		GLuint t;
		switch(type) {
		case VERTEX:
			t = GL_VERTEX_SHADER;
			break;
		case FRAGMENT:
			t = GL_FRAGMENT_SHADER;
			break;
		}

		_id = glCreateShader(t);
	}
	~Shader()
	{
		glDeleteShader(_id);
	}
	
	void loadSource(char *source, long size) {
		glShaderSource(_id, 1, &source, nullptr);
		_findVariables(source, size);
	}
	void loadSourceFromFile(const std::string &filename) throw(FileNotFoundException) {
		FileReader fr(filename);
		_name = filename;
		loadSource(fr.getData(), fr.getSize());
	}
	
private:
	/* returned array of chars must be deleted */
	char *_getCompilationLog() {
		int len = 0;
		int chars_written = 0;
		char *message = nullptr;
		glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &len);
		if(len > 1) {
			message = new char[len];
			glGetShaderInfoLog(_id, len, &chars_written, message);
		}
		return message;
	}
	
public:
	void compile() throw(Exception) {
		glCompileShader(_id);
		char *msg = _getCompilationLog();
		if(msg != nullptr) {
			fprintf(stderr, "Shader '%s' compile log:\n%s\n", _name.c_str(), msg);
			delete[] msg;
		}
		
		GLint st;
		glGetShaderiv(_id, GL_COMPILE_STATUS, &st);
		if(st != GL_TRUE) {
			throw Exception("Shader '" + _name + "' compile error");
		}
	}
	
	GLuint id() const {
		return _id;
	}
	const std::list<Variable> &attributes() const {
		return _attribs;
	}
	const std::list<Variable> &uniforms() const {
		return _uniforms;
	}
	
	void setName(const std::string &name) {
		_name = name;
	}
	std::string name() const {
		return _name;
	}
	
private:
	void _findVariables(char *source, long) {
		std::string string;
		std::smatch match;
		std::regex expr;
	
		string = std::string(source);
		expr = "(^|\n)[ \t]*attribute[ \t\n]*([^ \t\n]*)[ \t\n]*([^ \t\n;]*)[ \t\n]*;";
		while(std::regex_search(string, match, expr))
		{
			Variable var;
			var.name = match[3];
			var.type = match[2];
			_attribs.push_back(var);
			string = match.suffix().str();
		}
		
		string = std::string(source);
		expr = "(^|\n)[ \t]*uniform[ \t\n]*([^ \t\n]*)[ \t\n]*([^ \t\n;]*)[ \t\n]*;";
		while(std::regex_search(string, match, expr))
		{
			Variable var;
			var.name = match[3];
			var.type = match[2];
			_uniforms.push_back(var);
			string = match.suffix().str();
		}
	}
};
}
