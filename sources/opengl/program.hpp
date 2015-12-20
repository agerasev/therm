#pragma once

#include <cstring>

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
		Kind kind;
		Type type;
		int dim = 1;
		GLint id = -2;
	};
	struct AttribVariable : public Variable {
		VertexBuffer *buffer = nullptr;
	};
	struct UniformVariable : public Variable {
		union {
			float   fdata[16];
			int     idata[16];
		};
		const Texture *tex = nullptr;
		int texno = 0;
	};
private:
	GLuint _id;
	std::string _name;
	std::list<Shader*> _shaders;
	std::map<std::string, AttribVariable> _attribs;
	std::map<std::string, UniformVariable> _uniforms;
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
	
	GLuint id() const {
		return _id;
	}

	void setName(const std::string &name) {
		_name = name;
	}
	std::string name() const {
		return _name;
	}
	
private:
	void _parseType(const Shader::Variable &src_var, Variable &dst_var) {
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
		} else {
			throw Exception("Unknown type '" + src_var.type + "'");
		}
	}
	void _updateVariables() throw(Exception) {
		_attribs.clear();
		_uniforms.clear();
		for(Shader *s : _shaders) {
			for(int i = 0; i < 2; ++i) {
				for(const Shader::Variable &src_var : s->attributes()) {
					AttribVariable dst_var;
					_parseType(src_var, dst_var);
					_attribs.insert(std::pair<std::string, AttribVariable>(src_var.name, dst_var));
				}
			}
		}
		for(Shader *s : _shaders) {
			for(int i = 0; i < 2; ++i) {
				for(const Shader::Variable &src_var : s->uniforms()) {
					UniformVariable dst_var;
					_parseType(src_var, dst_var);
					_uniforms.insert(std::pair<std::string, UniformVariable>(src_var.name, dst_var));
				}
			}
		}
		int texno = 0;
		for(auto &pair : _uniforms) {
			if(pair.second.kind == Variable::SAMPLER) {
				pair.second.texno = texno;
				++texno;
			}
		}
	}
	void _getVariablesID() throw(Exception) {
		for(auto i = _attribs.begin(); i != _attribs.end(); ++i) {
			GLint loc = glGetAttribLocation(_id, i->first.c_str());
			if(loc == -1)
				throw Exception("Attrib '" + i->first + "' location error");
			i->second.id = loc;
		}
		for(auto i = _uniforms.begin(); i != _uniforms.end(); ++i) {
			GLint loc = glGetUniformLocation(_id, i->first.c_str());
			if(loc == -1)
				throw Exception("Uniform '" + i->first + "' location error");
			i->second.id = loc;
		}
	}

public:
	const std::map<std::string, AttribVariable> &attributes() const {
		return _attribs;
	}
	const std::map<std::string, UniformVariable> &uniforms() const {
		return _uniforms;
	}
	
private:
	void _loadUniform(const UniformVariable &var) {
		if(var.kind == Variable::SAMPLER && var.tex != nullptr) {
			glActiveTexture(GL_TEXTURE0 + var.texno);
			glUniform1i(var.id, var.texno);
			var.tex->bind();
			return;
		}
		switch(var.type) {
		case FLOAT:
			switch(var.kind) {
			case Variable::SCALAR:
				glUniform1fv(var.id, 1, var.fdata);
				break;
			case Variable::VECTOR:
				switch(var.dim) {
				case 2:
					glUniform2fv(var.id, 1, var.fdata);
					break;
				case 3:
					glUniform3fv(var.id, 1, var.fdata);
					break;
				case 4:
					glUniform4fv(var.id, 1, var.fdata);
					break;
				}
				break;
			case Variable::MATRIX:
				switch(var.dim) {
				case 2:
					glUniformMatrix2fv(var.id, 1, GL_FALSE, var.fdata);
					break;
				case 3:
					glUniformMatrix3fv(var.id, 1, GL_FALSE, var.fdata);
					break;
				case 4:
					glUniformMatrix4fv(var.id, 1, GL_FALSE, var.fdata);
					break;
				break;
				}
				break;
			default:
				break;
			}
			break;
		case INT:
			switch(var.kind) {
			case Variable::SCALAR:
				glUniform1iv(var.id, 1, var.idata);
				break;
			case Variable::VECTOR:
				switch(var.dim) {
				case 2:
					glUniform2iv(var.id, 1, var.idata);
					break;
				case 3:
					glUniform3iv(var.id, 1, var.idata);
					break;
				case 4:
					glUniform4iv(var.id, 1, var.idata);
					break;
				}
				break;
			default:
				break;
			}
			break;
		}
	}
	
public:
	void evaluate() {
		enable();
		
		for(const auto &p : _uniforms) {
			UniformVariable var = p.second;
			_loadUniform(var);
		}
		
		for(const auto &p : _attribs) {
			Variable var = p.second;
			glEnableVertexAttribArray(var.id);
		}
		
		for(const auto &p : _attribs) {
			AttribVariable var = p.second;
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
			AttribVariable var = p.second;
			VertexBuffer *buffer = var.buffer;
			if(buffer != nullptr) {
				buffer->draw();
			}
		}
		
		for(const auto &p : _attribs) {
			AttribVariable var = p.second;
			glDisableVertexAttribArray(var.id);
		}
		
		disable();
	}
	
	void setAttribute(const std::string &name, VertexBuffer *buf) {
		auto iter = _attribs.find(name);
		if(iter == _attribs.end())
			throw Exception("No such attribute '" + name + "'");
		AttribVariable &var = iter->second;
		if(var.type != buf->type())
			throw Exception("Attribute '" + name + "' type mismatch");
		var.buffer = buf;
	}
	
	template <typename T>
	void setUniform(const std::string &name, T *data, long len) throw(Exception) {
		auto iter = _uniforms.find(name);
		if(iter == _uniforms.end())
			throw Exception("No such uniform '" + name + "'");
		UniformVariable &var = iter->second;
		switch(var.kind) {
		case Variable::SCALAR:
			if(len != 1)
				throw Exception("Uniform '" + name + "' is scalar, but buffer size is " + std::to_string(len));
			break;
		case Variable::VECTOR:
			if(var.dim != len)
				throw Exception("Uniform '" + name + "' vector size mismatch");
			break;
		case Variable::MATRIX:
			if(var.dim*var.dim != len)
				throw Exception("Uniform '" + name + "' matrix size mismatch");
			break;
		case Variable::SAMPLER:
			throw Exception("Uniform '" + name + "' is sampler");
			break;
		}
		if(var.type != get_type<T>::value)
			throw Exception("Uniform '" + name + "' type mismatch");
		switch(var.type) {
		case INT:
			memcpy(var.idata, data, sizeof(int)*len);
			break;
		case FLOAT:
			memcpy(var.fdata, data, sizeof(float)*len);
			break;
		}
	}
	
	template <typename T>
	typename std::enable_if<std::is_arithmetic<T>::value, void>::type setUniform(const std::string &name, T data) {
		setUniform(name, &data, 1);
	}
	void setUniform(const std::string &name, const Texture *tex) {
		auto iter = _uniforms.find(name);
		if(iter == _uniforms.end())
			throw Exception("No such uniform '" + name + "'");
		UniformVariable &var = iter->second;
		if(var.kind != Variable::SAMPLER)
			throw Exception("Uniform '" + name + "' is not sampler");
		var.tex = tex;
	}
};
}
