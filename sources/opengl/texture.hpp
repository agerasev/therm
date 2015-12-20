#pragma once

#include <GL/glew.h>

#include <cstdio>

namespace gl {
class Texture {
public:
	enum Format {
		RGB,
		RGBA
	};
	enum Type {
		UBYTE,
		FLOAT
	};
	enum Interpolation {
		LINEAR,
		NEAREST
	};

private:
	GLuint _id = 0;
	int _width = 0, _height = 0;
	Format _format = RGB;
	Type _type = UBYTE;
	
public:
	Texture() {
		glGenTextures(1, &_id);
		
	}
	virtual ~Texture() {
		glDeleteTextures(1, &_id);
	}
	
	void bind() const {
		glBindTexture(GL_TEXTURE_2D, _id);
	}
	static void unbind() {
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	void loadData(const void *data, int width, int height, Format format, Type type, Interpolation inp = LINEAR) {
		bind();
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		setInterpolation(inp);
		
		GLuint ifmt;
		GLuint fmt, t;
		switch(format) {
		case RGB:
			fmt = GL_RGB;
			break;
		case RGBA:
			fmt = GL_RGBA;
			break;
		}
		switch(type) {
		case UBYTE:
			t = GL_UNSIGNED_BYTE;
			ifmt = fmt;
			break;
		case FLOAT:
			t = GL_FLOAT;
			ifmt = (format == RGB) ? GL_RGB32F : GL_RGBA32F;
			break;
		}
		
		glTexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, fmt, t, data);
		
		_width = width;
		_height = height;
		_format = format;
		_type = type;
	}
	
	void setInterpolation(Interpolation inp) const {
		bind();
		switch(inp) {
		case LINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;
		case NEAREST:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;
		}
	}

	GLuint id() const {
		return _id;
	}
	int width() const {
		return _width;
	}
	int height() const {
		return _height;
	}
	Type type() const {
		return _type;
	}
	Format format() const {
		return _format;
	}
};
}
