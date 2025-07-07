#include <heightmap.hpp>
#include <stb_image.h>


height_map::height_map() { _height = nullptr; }

height_map::height_map(int sizeX, int sizeZ)
{
	size[0] = sizeX; size[1] = sizeZ;
	_height = new float* [size[1]];
	for(int i = 0; i < size[1]; i++) _height[i] = new float[size[0]];
}

height_map::~height_map()
{
	for (int i = 0; i < size[1]; i++) delete[] _height[i];
	delete[] _height;
}

void height_map::set_value(float height, int x, int y)
{
	if (x < 0 || x >= size[0] || y < 0 || y > size[1]) xterminate("heightmap out of range.", QUI);

	_height[x][y] = height;
}

void height_map::set_bounding_rect(float minX, float minZ, float maxX, float maxZ)
{
	bounding_rect[0] = minX; bounding_rect[1] = minZ;
	bounding_rect[2] = maxX; bounding_rect[3] = maxZ;
}

void height_map::resize(int sizeX, int sizeY)
{
	size[0] = sizeX; size[1] = sizeY;

	for (int i = 0; i < size[1]; i++) delete[] _height[i];
	delete[] _height;

	_height = new float* [size[1]];
	for (int i = 0; i < size[1]; i++) _height[i] = new float[size[0]];
}

void height_map::load_from_file(std::string filename)
{
	int comp;
	unsigned char* data = stbi_load(filename.c_str(), &size[0], &size[1], &comp, 1);

	_height = new float* [size[1]];
	for (int i = 0; i < size[1]; i++) _height[i] = new float[size[0]];

	if (data == NULL)
	{
		std::string e = stbi_failure_reason();
		e += "\n" + filename + "\n";
		xterminate(e.c_str(), QUI);
	}

	for(int iz = 0; iz < size[1]; iz++)
		for (int ix = 0; ix < size[0]; ix++)
		{
			_height[iz][ix] = data[(size[0] - 1 - iz) * size[1] + ix] / 50.f; // / 50.f
		}
}

renderable height_map::to_renderable(renderable& r)
{
	std::vector<unsigned int> buffer_id;

	std::vector<float> height_map_3d;
	std::vector<float> normals;
	std::vector<float> tex_coords;

	for (unsigned int iz = 0; iz < size[1]; iz++)
		for (unsigned int ix = 0; ix < size[0]; ix++)
		{
			height_map_3d.push_back(bounding_rect[0] + (ix / float(size[0])) * bounding_rect[2]);
			height_map_3d.push_back(_height[ix][iz]);
			height_map_3d.push_back(bounding_rect[1] + (iz / float(size[1])) * bounding_rect[3]);

			// Normale
			float h_current = _height[ix][iz];
			float h_right, h_forward;

			if (ix < size[0] - 1) h_right = _height[ix + 1][iz];
			else h_right = h_current;

			if (iz < size[1] - 1) h_forward = _height[ix][iz + 1];
			else h_forward = h_current;
						
			glm::vec3 tangentX = glm::vec3(1.0f, h_right - h_current, 0.0f);
			glm::vec3 tangentZ = glm::vec3(0.0f, h_forward - h_current, 1.0f);
			glm::vec3 normal = glm::cross(tangentZ, tangentX);

			normals.push_back(normal.x);
			normals.push_back(normal.y);
			normals.push_back(normal.z);


			tex_coords.push_back(ix);
			tex_coords.push_back(iz);

			if (iz >= size[1] - 1 || ix >= size[0] - 1) continue;

			buffer_id.push_back((iz * size[1]) + ix);
			buffer_id.push_back((iz * size[1]) + ix + 1);
			buffer_id.push_back((iz + 1) * size[1] + ix + 1);

			buffer_id.push_back((iz * size[1]) + ix);
			buffer_id.push_back((iz + 1) * size[1] + ix + 1);
			buffer_id.push_back((iz + 1) * size[1] + ix);
		}

	r.add_vertex_attribute<float>(&height_map_3d[0], size[0] * size[1] * 3, 0, 3);
	r.add_vertex_attribute<float>(&normals[0], size[0] * size[1] * 3, 2, 3);
	r.add_vertex_attribute<float>(&tex_coords[0], size[0] * size[1] * 2, 4, 2);
	r.add_indices<unsigned int>(&buffer_id[0], static_cast<unsigned int>(buffer_id.size()), GL_TRIANGLES);
	
	return r;
}
