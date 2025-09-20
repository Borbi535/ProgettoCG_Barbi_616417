#pragma once

#include <GL/glew.h>
#include <iostream>
#include <vector>
#include <debugging.h>
#include <xerrori.hpp>

#define MAX_SHADOWMAP_TEXARRAY_LAYERS 128


struct frame_buffer_object
{
	int w, h;
	GLuint id_fbo, id_tex, id_tex1, id_depth;
	bool use_texture_for_depth;

	frame_buffer_object(int w_, int h_, bool _use_texture_for_depth = false);
	~frame_buffer_object();

private:
	void check(int fboStatus);
};

struct shadowmap_texture_array
{
	GLuint id_tex;
	int size_x, size_y;

	shadowmap_texture_array(int size_x, int size_y);

	void update(std::vector<std::shared_ptr<frame_buffer_object>> fbos);

};