#include "graphics.hpp"

#include <cstdio>

#include "GL/glew.h"

#include "opengl/program.hpp"

Graphics::Graphics() {
	gl::Shader shader(gl::Shader::VERTEX);
	shader.loadSourceFromFile("shaders/place.vert");
	shader.compile();
	
	gl::Program program;
	program.attach(&shader);
	program.link();
	const gl::Program::VarMap *(maps[2]) = {&program.getAttributes(), &program.getUniforms()};
	for(int i = 0; i < 2; ++i) {
		for(const std::pair<std::string, gl::Program::Variable> &var_pair : *(maps[i])) {
			std::string type_name;
			switch(var_pair.second.type) {
			case gl::Program::Variable::FLOAT:
				type_name = "float";
				break;
			case gl::Program::Variable::INT:
				type_name = "int";
				break;
			}
			printf("variable '%s': id=%d, type=%s, size=%d;\n", var_pair.first.c_str(), var_pair.second.id, type_name.c_str(), var_pair.second.size);
		}
	}
	glClearColor(0.0f,0.0f,0.0f,1.0f);
}

Graphics::~Graphics() {
	
}

void Graphics::render() {
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
}
