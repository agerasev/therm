#pragma once

#include <GL/glew.h>

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
	
	void loadData(const void *data, int width, int height, Format format, Type type) {
		bind();
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
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
		
		unbind();
		
		_width = width;
		_height = height;
		_format = format;
		_type = type;
	}
	
	GLuint getID() {
		return _id;
	}
	int getWidth() const {
		return _width;
	}
	int getHeight() const {
		return _height;
	}
	Type getType() const {
		return _type;
	}
	Format getFormat() const {
		return _format;
	}
};
}
