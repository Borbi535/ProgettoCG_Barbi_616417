#pragma once

#include <matrix_stack.h>
#include <shaders.h>
#include <xerrori.hpp>
//#include <renderable.h>
#include <simple_shapes.h>

extern shader texture_shader;

class GraphicalDebugObject
{
public:
	GraphicalDebugObject(Shapes shape_type);
	
	renderable GetRenderable();

	void Draw(matrix_stack& stack, shader& active_shader, float scale = 1.f);

private:
	renderable shape;
};