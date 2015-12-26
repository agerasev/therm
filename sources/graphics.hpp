#pragma once

#include <cstdio>

#include <string>
#include <vector>
#include <map>
#include <functional>

#include <GL/glew.h>

#include "opengl/program.hpp"
#include "opengl/framebuffer.hpp"

class Graphics {
private:
	int width = 0, height = 0;
	std::map<std::string, gl::Shader*> shaders;
	std::map<std::string, gl::Program*> programs;
	gl::VertexBuffer buf;
	gl::Texture tex;
	gl::FrameBuffer *(fb[2]);
	
	struct ShaderInfo {
		std::string name;
		std::string path;
		gl::Shader::Type type;
		ShaderInfo(const std::string &n, const std::string &p, gl::Shader::Type t)
		  : name(n), path(p), type(t) {}
	};
	
	struct ProgramInfo {
		std::string name;
		std::string vert, frag;
		ProgramInfo(const std::string &n, const std::string &v, const std::string &f)
		  : name(n), vert(v), frag(f) {}
	};
	
	void swapBuffers() {
		gl::FrameBuffer *tmp = fb[0];
		fb[0] = fb[1];
		fb[1] = tmp;
	}

public:
	Graphics()
	{
		std::vector<ShaderInfo> shader_info({
		  ShaderInfo("position",  "shaders/position.vert",  gl::Shader::VERTEX),
		  ShaderInfo("draw",      "shaders/draw.frag",      gl::Shader::FRAGMENT),
		  ShaderInfo("texture",   "shaders/texture.frag",   gl::Shader::FRAGMENT),
		  ShaderInfo("diffuse",   "shaders/diffuse.frag",   gl::Shader::FRAGMENT)
		});
		
		for(const ShaderInfo &info : shader_info) {
			gl::Shader *shader = new gl::Shader(info.type);
			shader->setName(info.name);
			shader->loadSourceFromFile(info.path);
			shader->compile();
			shaders.insert(std::pair<std::string, gl::Shader*>(info.name, shader));
		}
		
		std::vector<ProgramInfo> program_info({
		  ProgramInfo("draw",    "position", "draw"),
		  ProgramInfo("texture", "position", "texture"),
		  ProgramInfo("diffuse", "position", "diffuse")
		});
		
		for(const ProgramInfo &info : program_info) {
			gl::Program *prog = new gl::Program();
			prog->setName(info.name);
			prog->attach(shaders[info.vert]);
			prog->attach(shaders[info.frag]);
			prog->link();
			programs.insert(std::pair<std::string, gl::Program*>(info.name, prog));
		}
		
		fb[0] = new gl::FrameBuffer();
		fb[1] = new gl::FrameBuffer();
		
		float vertex_data[] = {
		  0, 0, 1, 0, 0, 1,
		  0, 1, 1, 0, 1, 1
		};
		buf.loadData(vertex_data, 12);
		
		float map_data[]    = {2, 0, 0, 2};
		float offset_data[] = {-1, -1};
		
		static const int sx = 256, sy = 256;
		int area_size_data[] = {sx, sy};
		
		programs["draw"]->setAttribute("a_vertex", &buf);
		programs["draw"]->setUniform("u_map", map_data, 4);
		programs["draw"]->setUniform("u_offset", offset_data, 2);
		
		programs["texture"]->setAttribute("a_vertex", &buf);
		programs["texture"]->setUniform("u_map", map_data, 4);
		programs["texture"]->setUniform("u_offset", offset_data, 2);
		programs["texture"]->setUniform("u_texture", &tex);
		
		programs["diffuse"]->setAttribute("a_vertex", &buf);
		programs["diffuse"]->setUniform("u_map", map_data, 4);
		programs["diffuse"]->setUniform("u_offset", offset_data, 2);
		programs["diffuse"]->setUniform("u_area_size", area_size_data, 2);
		
		double ir = 0.4;
		std::function<double(double)> 
		inner = [](double a) {
			return 0.5*(1.0 + cos(2*a));
		}, 
		outer = [](double a) {
			return 0.5*(1.0 - sin(2*a));
		};
		float *data = new float[3*sx*sy];
		for(int iy = 0; iy < sy; ++iy) {
			for(int ix = 0; ix < sx; ++ix) {
				double x = 2.1*(double(ix)/sx - 0.5), y = 2.1*(double(iy)/sy - 0.5);
				double r = sqrt(x*x + y*y);
				double a = atan2(y, x);
				if(r > 1.0) {
					data[3*(iy*sx + ix) + 0] = outer(a);
					data[3*(iy*sx + ix) + 1] = 0.0;
				} else if(r < ir) {
					data[3*(iy*sx + ix) + 0] = inner(a);
					data[3*(iy*sx + ix) + 1] = 0.0;
				} else {
					data[3*(iy*sx + ix) + 0] = 0.0;
					data[3*(iy*sx + ix) + 1] = 1.0;
				}
				data[3*(iy*sx + ix) + 2] = 0.0;
			}
		}
		tex.loadData(data, sx, sy, gl::Texture::RGB, gl::Texture::FLOAT, gl::Texture::NEAREST);
		delete[] data;
		
		fb[0]->setSize(sx, sy);
		fb[1]->setSize(sx, sy);
		fb[0]->getTexture()->setInterpolation(gl::Texture::NEAREST);
		fb[1]->getTexture()->setInterpolation(gl::Texture::NEAREST);
		
		fb[0]->bind();
		programs["texture"]->setUniform("u_texture", &tex);
		programs["texture"]->evaluate();
		gl::FrameBuffer::unbind();
		
		glClearColor(0.0f,0.0f,0.0f,1.0f);
	}
	
	~Graphics() {
		for(const auto &p : programs) {
			delete p.second;
		}
		for(const auto &p : shaders) {
			delete p.second;
		}
	}
	
	void resize(int w, int h) {
		width = w;
		height = h;
		glViewport(0, 0, width, height);
	}
	
	void render() {
		glClear(GL_COLOR_BUFFER_BIT);
		
		for(int i = 0; i < 0x80; ++i) {
			fb[1]->bind();
			programs["diffuse"]->setUniform("u_source", fb[0]->getTexture());
			programs["diffuse"]->evaluate();
			gl::FrameBuffer::unbind();
			swapBuffers();
		}
		
		glViewport(0, 0, width, height);
		fb[0]->getTexture()->setInterpolation(gl::Texture::LINEAR);
		programs["draw"]->setUniform("u_texture", fb[0]->getTexture());
		programs["draw"]->evaluate();
		fb[0]->getTexture()->setInterpolation(gl::Texture::NEAREST);
		glFlush();
	}
	
	void writeFile(const std::string &fn) {
		FILE *f = fopen(fn.c_str(), "w");
		if(f == nullptr) {
			perror("error write file");
			return;
		}
		const gl::Texture *t = fb[0]->getTexture();
		int sx = t->width(), sy = t->height();
		float *data = new float[3*sx*sy];
		fb[0]->bind();
		glReadPixels(0, 0, sx, sy, GL_RGB, GL_FLOAT, data);
		for(int iy = 0; iy < sy; iy+=4) {
			for(int ix = 0; ix < sx; ix+=4) {
				fprintf(f, "%f ", data[3*(sx*iy + ix) + 0]);
			}
			fprintf(f, "\n");
		}
		gl::FrameBuffer::unbind();
		delete[] data;
		fclose(f);
	}
};
