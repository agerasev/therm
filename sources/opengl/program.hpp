#pragma once

#include <list>
#include <map>
#include <string>
#include <regex>

#include "exception.hpp"
#include "shader.hpp"

namespace gl {
class Program {
public:
	struct Variable {
		enum Type {
			INT,
			FLOAT
		};
		GLint id;
		Type type;
		int size;
	};
	typedef std::map<std::string, Variable> VarMap;
private:
	GLuint _id;
	std::string _name;
	std::list<Shader*> shaders;
	VarMap attribs;
	VarMap uniforms;
public:
	Program() {
		_id = glCreateProgram();
	}
	~Program() {
		for(Shader *s : shaders) {
			glDetachShader(_id, s->getID());
		}
		glDeleteProgram(_id);
	}
	
	void attach(Shader *s) throw(Exception) {
		shaders.push_back(s);
		_updateVariables();
		glAttachShader(_id, s->getID());
	}
	void detach(Shader *s) throw(Exception) {
		glDetachShader(_id, s->getID());
		shaders.remove(s);
		_updateVariables();
	}
	
	void link() throw(Exception) {
		int link_ok;
		glLinkProgram(_id);
		glGetProgramiv(_id, GL_LINK_STATUS, &link_ok);
		if(!link_ok) {
			throw Exception("Program '" + _name + "' link error");
		}
		_getVariablesID();
	}

	void enable() {
		glUseProgram(_id);
	}
	static void disable() {
		glUseProgram(0);
	}
	
	void setName(const std::string &name) {
		_name = name;
	}
	std::string getName() const {
		return _name;
	}
	
private:
	void _updateVariables() throw(Exception) {
		attribs.clear();
		uniforms.clear();
		VarMap *(dst_map[2]) = {&attribs, &uniforms};
		for(Shader *s : shaders) {
			const std::list<Shader::Variable> *(src_list[2]) = {&s->getAttributes(), &s->getUniforms()};
			for(int i = 0; i < 2; ++i) {
				for(const Shader::Variable &src_var : *(src_list[i])) {
					Variable dst_var;
					std::smatch match;
					std::regex expr("^([^0123456789]*)([1234567890]*)$");
					if(std::regex_search(src_var.type, match, expr)) {
						std::string dim_str(match[2]);
						if(dim_str.length() < 1)
							dst_var.size = 1;
						else
							dst_var.size = std::stoi(dim_str);
						std::string type(match[1]);
						if(type == "float" || type == "vec" || type == "mat") {
							dst_var.type = Variable::FLOAT;
						} else if(type == "int" || type == "ivec" || type == "imat") {
							dst_var.type = Variable::INT;
						} else
							throw Exception("Unknown type '" + type + "'");
						if(type == "mat" || type == "imat")
							dst_var.size *= dst_var.size;
					} else {
						throw Exception("Unknown type '" + src_var.type + "'");
					}
					dst_map[i]->insert(std::pair<std::string, Variable>(src_var.name, dst_var));
				}
			}
		}
	}
	void _getVariablesID() throw(Exception) {
		for(VarMap::iterator i = attribs.begin(); i != attribs.end(); ++i) {
			GLint loc = glGetAttribLocation(_id, i->first.c_str());
			if(loc == -1)
				throw Exception("Attrib '" + i->first + "' location error");
			i->second.id = loc;
		}
		for(VarMap::iterator i = uniforms.begin(); i != uniforms.end(); ++i) {
			GLint loc = glGetUniformLocation(_id, i->first.c_str());
			if(loc == -1)
				throw Exception("Uniform '" + i->first + "' location error");
			i->second.id = loc;
		}
	}

public:
	const VarMap &getAttributes() const {
		return attribs;
	}
	const VarMap &getUniforms() const {
		return uniforms;
	}
};
}
