#pragma once

#include <iostream>
#include <glm.hpp>
#include <renderable.h>
#include <xerrori.hpp>


struct height_map
{
	private:
		int size[2];
		float** _height;

		/// rectangle in xz where the terrain is located {minx,minz,sizex,sizez}
		float bounding_rect[4];


	public:
		height_map();
		height_map(int sizeX, int sizeY);
		~height_map();
		void set_value(float height, int x, int y);
		void set_bounding_rect(float minX, float minZ, float maxX, float maxZ);
		void resize(int sizeX, int sizeY);
		void load_from_file(std::string filename);
		renderable to_renderable(renderable& r);
};