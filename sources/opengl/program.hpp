#pragma once

#include <list>
#include <map>
#include <string>
#include <regex>

#include "type.hpp"
#include "exception.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vertexbuffer.hpp"

namespace gl {
class Program {
public:
	struct Variable {
		enum Kind {
			SCALAR,
			VECTOR,
			MATRIX,
			SAMPLER
		};
		/* TODO: use different types of variables for attribs and uniforms */
		VertexBuffer *buffer = nullptr;
		GLint id = -2;
		Kind kind;
		Type type;
		int dim = 1;
		int sampler = 0;
	};
	typedef std::map<std::string, Variable> VarMap;
private:
	GLuint _id;
	std::string _name;
	std::list<Shader*> _shaders;
	VarMap _attribs;
	VarMap _uniforms;
public:
	Program() {
		_id = glCreateProgram();
	}
	~Program() {
		for(Shader *s : _shaders) {
			glDetachShader(_id, s->id());
		}
		glDeleteProgram(_id);
	}
	
	void attach(Shader *s) throw(Exception) {
		_shaders.push_back(s);
		_updateVariables();
		glAttachShader(_id, s->id());
	}
	void detach(Shader *s) throw(Exception) {
		glDetachShader(_id, s->id());
		_shaders.remove(s);
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
	
	GLuint id() {
		return _id;
	}

	void setName(const std::string &name) {
		_name = name;
	}
	std::string name() const {
		return _name;
	}
	
private:
	void _updateVariables() throw(Exception) {
		_attribs.clear();
		_uniforms.clear();
		int smp_cnt = 0;
		VarMap *(dst_map[2]) = {&_attribs, &_uniforms};
		for(Shader *s : _shaders) {
			const std::list<Shader::Variable> *(src_list[2]) = {&s->attributes(), &s->uniforms()};
			for(int i = 0; i < 2; ++i) {
				for(const Shader::Variable &src_var : *(src_list[i])) {
					Variable dst_var;
					std::smatch match;
					std::regex vec_expr("^([^0123456789]*)([1234567890]*)$");
					std::regex smp_expr("^sampler([1234567890])D$");
					if(std::regex_search(src_var.type, match, vec_expr)) {
						std::string dim_str(match[2]);
						dst_var.dim = dim_str.length() < 1 ? 1 : std::stoi(dim_str);
						
						std::string type(match[1]);
						if(type == "float" || type == "vec" || type == "mat") {
							dst_var.type = FLOAT;
						} else if(type == "int" || type == "ivec") {
							dst_var.type = INT;
						} else
							throw Exception("Unknown type '" + type + "'");
						
						if(type == "float" || type == "int")
							dst_var.kind = Variable::SCALAR;
						else if(type == "vec" || type == "ivec")
							dst_var.kind = Variable::VECTOR;
						else if(type == "mat")
							dst_var.kind = Variable::MATRIX;
						
					} else if(std::regex_search(src_var.type, match, smp_expr)) {
						dst_var.dim = std::stoi(std::string(match[1]));
						dst_var.kind = Variable::SAMPLER;
						dst_var.sampler = smp_cnt++;
					} else {
						throw Exception("Unknown type '" + src_var.type + "'");
					}
					dst_map[i]->insert(std::pair<std::string, Variable>(src_var.name, dst_var));
				}
			}
		}
	}
	void _getVariablesID() throw(Exception) {
		for(VarMap::iterator i = _attribs.begin(); i != _attribs.end(); ++i) {
			GLint loc = glGetAttribLocation(_id, i->first.c_str());
			if(loc == -1)
				throw Exception("Attrib '" + i->first + "' location error");
			i->second.id = loc;
		}
		for(VarMap::iterator i = _uniforms.begin(); i != _uniforms.end(); ++i) {
			GLint loc = glGetUniformLocation(_id, i->first.c_str());
			if(loc == -1)
				throw Exception("Uniform '" + i->first + "' location error");
			i->second.id = loc;
		}
	}

public:
	const VarMap &attributes() const {
		return _attribs;
	}
	const VarMap &uniforms() const {
		return _uniforms;
	}
	
	void evaluate() {
		enable();
		
		for(const auto &p : _attribs) {
			Variable var = p.second;
			glEnableVertexAttribArray(var.id);
		}
		
		for(const auto &p : _attribs) {
			Variable var = p.second;
			VertexBuffer *buffer = var.buffer;
			if(buffer != nullptr) {
				GLuint glt;
				switch(var.type) {
				case FLOAT:
					glt = GL_FLOAT;
					break;
				case INT:
					glt = GL_INT;
					break;
				}
				buffer->bind();
				glVertexAttribPointer(var.id, var.dim, glt, GL_FALSE, 0, NULL);
				buffer->unbind();
			}
		}
		
		for(const auto &p : _attribs) {
			Variable var = p.second;
			VertexBuffer *buffer = var.buffer;
			if(buffer != nullptr) {
				buffer->draw();
			}
		}
		
		for(const auto &p : _attribs) {
			Variable var = p.second;
			glDisableVertexAttribArray(var.id);
		}
		
		disable();
	}
	
	void setAttribute(const std::string &name, VertexBuffer *buf) {
		auto iter = _uniforms.find(name);
		if(iter == _uniforms.end())
			throw Exception("No such attribute '" + name + "'");
		Variable &var = iter->second;
		if(var.type != buf->type())
			throw Exception("Attribute '" + name + "' type mismatch");
		var.buffer = buf;
	}
	
	template <typename T>
	void setUniform(const std::string &name, T *data, long len) throw(Exception) {
		auto iter = _uniforms.find(name);
		if(iter == _uniforms.end())
			throw Exception("No such uniform '" + name + "'");
		Variable var = iter->second;
		if(var.kind == Variable::SAMPLER)
			throw Exception("Uniform '" + name + "' is sampler");
		if(get_type<T>::value != var.type)
			throw Exception("Uniform '" + name + "' type mismatch");
		switch(var.kind) {
		case Variable::SCALAR:
			if(len != 1)
				throw Exception("Uniform '" + name + "' is scalar, but buffer size is " + std::to_string(len));
			switch(var.type) {
			case INT:
				glUniform1iv(var.id, 1, data);
				break;
			case FLOAT:
				glUniform1fv(var.id, 1, data);
				break;
			}
			break;
		case Variable::VECTOR:
			if(var.dim != len)
				throw Exception("Uniform '" + name + "' vector size mismatch");
			switch(var.type) {
			case INT:
				switch(var.dim) {
				case 2:
					glUniform2iv(var.id, 1, data);
					break;
				case 3:
					glUniform3iv(var.id, 1, data);
					break;
				case 4:
					glUniform4iv(var.id, 1, data);
					break;
				}
				break;
			case FLOAT:
				switch(var.dim) {
				case 2:
					glUniform2fv(var.id, 1, data);
					break;
				case 3:
					glUniform3fv(var.id, 1, data);
					break;
				case 4:
					glUniform4fv(var.id, 1, data);
					break;
				}
				break;
			}
			break;
		case Variable::MATRIX:
			if(var.dim*var.dim != len)
				throw Exception("Uniform '" + name + "' matrix size mismatch");
			if(var.type != FLOAT)
				throw Exception("Uniform '" + name + "' is non-float matrix");
			switch(var.dim) {
			case 2:
				glUniformMatrix2fv(var.id, 1, data);
				break;
			case 3:
				glUniformMatrix3fv(var.id, 1, data);
				break;
			case 4:
				glUniformMatrix4fv(var.id, 1, data);
				break;
			break;
			}
		}
	}
	template <typename T>
	void setUniform(const std::string &name, T data) {
		setUniform(name, &data, 1);
	}
	void setUniform(const std::string &name, Texture *tex) {
		
	}
};
}
