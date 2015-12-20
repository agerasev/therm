#pragma once

#include <type_traits>

#include <GL/glew.h>

#include "type.hpp"

namespace gl {
class VertexBuffer {
public:
private:
	GLuint _id;
	long _size;
	Type _type;
public:
	VertexBuffer() {
		glGenBuffers(1, &_id);
	}
	~VertexBuffer() {
		glDeleteBuffers(1, &_id);
	}
	
	void bind() {
		glBindBuffer(GL_ARRAY_BUFFER, _id);
	}
	static void unbind() {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
	template <typename T>
	void loadData(T *data, long size) {
		bind();
		_size = size;
		_type = get_type<T>::value;
		glBufferData(GL_ARRAY_BUFFER, size*sizeof(T), data, GL_STATIC_DRAW);
		unbind();
	}
	
	void draw() {
		glDrawArrays(GL_TRIANGLES, 0, _size);
	}
	
	GLuint id() const {
		return _id;
	}
	long size() const {
		return _size;
	}
	Type type() {
		return _type;
	}
};
}
