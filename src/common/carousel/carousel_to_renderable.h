#pragma once

#include "renderable.h"
#include "carousel.h"
#include "stuff.h"



struct game_to_renderable 
{	
	template <typename T>
	static void ct(T* dst, glm::vec3 src)
	{
		dst[0] = src.x;
		dst[1] = src.y;
		dst[2] = src.z;
	}
	static void to_track(const race & r, renderable& r_t) 
	{
		std::vector<float> buffer_pos;
		buffer_pos.resize(r.t().curbs[0].size() * 2 * 3);
		
		for (unsigned int i = 0; i < r.t().curbs[0].size(); ++i)
		{
			ct(&buffer_pos[(2 * i) * 3], r.t().curbs[0][i % (r.t().curbs[0].size())]);
			ct(&buffer_pos[(2 * i + 1) * 3], r.t().curbs[1][i % (r.t().curbs[1].size())]);
		}

		std::vector<float> tex_coords;
		tex_coords.resize(r.t().curbs[0].size() * 2 * 2);
		std::vector<unsigned int> index_buffer;
		index_buffer.resize(r.t().curbs[0].size() * 2);
		double v = 0.f; // offset coordinata v da un punto all'altro
		int j = 0; double a, b, c, hb;
		a = b = c = hb = -1;
		for (unsigned int i = 0; i < buffer_pos.size(); i += 3)
		{
			index_buffer[i / 3] = i / 3;

			tex_coords[j] = (i / 3) % 2;
			tex_coords[j + 1] = v;
			j += 2;

			// per gli ultimi 2 punti finchè non trovo una soluzione migliore la coordinata v aumenta della stessa quantità calcolata per il terzultimo
			if (i < buffer_pos.size() - 6)
			{
				// lunghezza lato sul bordo del circuito  (il più corto) del triangolo p[i,i+1,i+2], p[i+3,i+4,i+5], p[i+6,i+7,i+8]
				b = glm::length(glm::vec3(buffer_pos[i + 6], buffer_pos[i + 7], buffer_pos[i + 8]) - glm::vec3(buffer_pos[i], buffer_pos[i + 1], buffer_pos[i + 2]));
				// le lunghezze degli altri 2 lati (seguendo la direzione del circuito, a è il lato piu in basso, b quell'altro)
				a = glm::length(glm::vec3(buffer_pos[i + 3], buffer_pos[i + 4], buffer_pos[i + 5]) - glm::vec3(buffer_pos[i], buffer_pos[i + 1], buffer_pos[i + 2]));
				c = glm::length(glm::vec3(buffer_pos[i + 6], buffer_pos[i + 7], buffer_pos[i + 8]) - glm::vec3(buffer_pos[i + 3], buffer_pos[i + 4], buffer_pos[i + 5]));

				// altezza relativa al lato b
				hb = (2 * hero(a, b, c)) / b;
			}

			//std::cout << "j = " << j << "size = " << r.t().curbs[0].size() * 4 << std::endl;
			//std::cout << "i = " << i << "size = " << buffer_pos.size() << std::endl;
			v += pythagoras(a, hb, true);
		}

		r_t.add_vertex_attribute<float>(&buffer_pos[0], static_cast<unsigned int>(buffer_pos.size()), 0, 3);
		r_t.add_vertex_attribute<float>(&tex_coords[0], r.t().curbs[0].size() * 2 * 2, 4, 2);
		r_t.add_indices<unsigned int>(&index_buffer[0], index_buffer.size(), GL_TRIANGLE_STRIP);
	}

	static void to_stick_object(const std::vector<stick_object>& vec, renderable& r_t)
	{

		std::vector<float> buffer_pos;
		buffer_pos.resize((vec.size()*2) * 3 );
		for (unsigned int i = 0; i < vec.size();++i)
		{
			ct(&buffer_pos[(2 * i) * 3], vec[i].pos);
			ct(&buffer_pos[(2 * i+1) * 3], vec[i].pos+glm::vec3(0, vec[i].height,0));
		}

		r_t.add_vertex_attribute<float>(&buffer_pos[0], static_cast<unsigned int>(buffer_pos.size()), 0, 3);
	}

	static void to_tree(const race& r, renderable& r_t)
	{
		to_stick_object(r.trees(), r_t);
	}

	static void to_lamps(const race& r, renderable& r_t)
	{
		to_stick_object(r.lamps(), r_t);
	}




	static void to_heightfield(const race& r, renderable& r_hf)
	{
		std::vector<unsigned int> buffer_id;
		// dimensioni dell'immagine dei valori di altezza
		const unsigned int& Z = static_cast<unsigned int>(r.ter().size_pix[1]);
		const unsigned int& X = static_cast<unsigned int>(r.ter().size_pix[0]);

		terrain ter = r.ter();

		std::vector<float> height_field_3d;
		std::vector<float> tex_coords;
		for (unsigned int iz = 0; iz < Z; ++iz)
			for (unsigned int ix = 0; ix < X; ++ix)
			{
				// pusho le coordinate X Y Z
				height_field_3d.push_back(ter.rect_xz[0] + (ix / float(X)) * ter.rect_xz[2]);
				height_field_3d.push_back(r.ter().hf(ix, iz));
				height_field_3d.push_back(ter.rect_xz[1] + (iz / float(Z)) * ter.rect_xz[3]);

				// pusho le coordinate texel
				tex_coords.push_back(ix);
				tex_coords.push_back(iz);
			}

		for (unsigned int iz = 0; iz < Z-1; ++iz)
			for (unsigned int ix = 0; ix < X-1; ++ix)
			{
				
				buffer_id.push_back((iz * Z) + ix);
				buffer_id.push_back((iz * Z) + ix + 1);
				buffer_id.push_back((iz + 1) * Z + ix + 1);

				buffer_id.push_back((iz * Z) + ix);
				buffer_id.push_back((iz + 1) * Z + ix + 1);
				buffer_id.push_back((iz + 1) * Z + ix);
			}

		r_hf.add_vertex_attribute<float>(&height_field_3d[0], X * Z * 3, 0, 3);
		r_hf.add_vertex_attribute<float>(&tex_coords[0], X * Z * 2, 4, 2);
		r_hf.add_indices<unsigned int>(&buffer_id[0], static_cast<unsigned int>(buffer_id.size()), GL_TRIANGLES);
	}
};