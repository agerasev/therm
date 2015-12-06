#pragma once

#include <cstdio>

#include "GL/glew.h"

#include "opengl/program.hpp"

class Graphics {
private:
	gl::Shader vert, frag;
	gl::Program prog;
	gl::VertexBuffer buf;
public:
	Graphics()
	    : vert(gl::Shader::VERTEX), frag(gl::Shader::FRAGMENT)
	{
		vert.loadSourceFromFile("shaders/place.vert");
		vert.compile();
		frag.loadSourceFromFile("shaders/draw.frag");
		frag.compile();
		
		prog.attach(&vert);
		prog.attach(&frag);
		prog.link();
		
		float vertex_data[] = {
		  0, 0, 1, 0, 0, 1,
		  0, 1, 1, 0, 1, 1
		};
		buf.loadData(vertex_data, 12);
		prog.setAttribute("vertex", &buf);
		
		float transform_data[] = {2, 0, 0, 2};
		prog.setUniform("transform", transform_data, 4);
		
		float translate_data[] = {-1, -1};
		prog.setUniform("translate", translate_data, 2);
		
		glClearColor(0.0f,0.0f,0.0f,1.0f);
	}
	
	~Graphics() {
		
	}
	
	void render() {
		glClear(GL_COLOR_BUFFER_BIT);
		prog.evaluate();
		glFlush();
	}
	
};
