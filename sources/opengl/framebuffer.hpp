#pragma once

#include <GL/glew.h>

#include "texture.hpp"
#include "exception.hpp"

namespace gl {
class FrameBuffer {
private:
	GLuint _id;
	Texture _tex;
	int _width, _height;
	
public:
	FrameBuffer() throw(Exception) {
		glGenFramebuffers(1, &_id);
	}
	virtual ~FrameBuffer() {
		glDeleteFramebuffers(1, &_id);
	}
	
	void setSize(int width, int height) throw(Exception) {
		_width = width;
		_height = height;
		
		bind();
		
		_tex.loadData(nullptr, width, height, Texture::RGBA, Texture::FLOAT);
		
		if(GLEW_VERSION_3_2)
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _tex.id(), 0);
		else if(GLEW_ARB_framebuffer_object)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _tex.id(), 0);
		else
			throw Exception("No framebuffer support : (GLEW_VERSION_3_2 || GLEW_ARB_framebuffer_object) == 0");
		
		GLenum bufs[1] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, bufs);
		
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			throw Exception("FrameBuffer create error");
	}
	
	void bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, _id);
		glViewport(0, 0, _width, _height);
	}
	static void unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	GLuint id() const {
		return _id;
	}
	const Texture *getTexture() const {
		return &_tex;
	}
};
}
