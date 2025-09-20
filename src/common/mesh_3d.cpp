#include <mesh_3d.hpp>

shader* Mesh3D::_shader = nullptr;

Mesh3D::Mesh3D() {}

Mesh3D::Mesh3D(std::string filename, float scale) : filename(filename), n_vert(0), n_tri(0), scale(scale)
{
	tinygltf::TinyGLTF loader;
	std::string err, warn;
	
	std::string ext = GetFilePathExtension(filename);
	
	bool ret = false;
	if (ext.compare("glb") == 0) 
	{
		// assume binary glTF.
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
	}
	else
	{
		// assume ascii glTF.
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
	}

	if (!warn.empty()) std::cout << "Warning: " << warn << std::endl;

	if (!err.empty()) std::cout << "Error: " << warn << std::endl;

	if (!ret)
	{
		std::string errmsg = "Failed to load "; errmsg += filename + ".gltf";
		xterminate(errmsg.c_str(), QUI);
	}

	CreateRenderable();
	aabb = AABB(bbox.min, bbox.max);
}

Mesh3D::Mesh3D(const Mesh3D& mesh)
{
	_shader = mesh._shader;
	model = mesh.model;
	object = mesh.object;
	bbox = mesh.bbox;
	id_textures = mesh.id_textures;
	n_vert = mesh.n_vert;
	n_tri = mesh.n_tri;
	scale = mesh.scale;
	filename = mesh.filename;
}

Mesh3D::~Mesh3D()
{
	;
}

AABB Mesh3D::GetAABB() const
{
	return aabb;
}

void Mesh3D::SetShader(shader& new_shader) { Mesh3D::_shader = &new_shader; }

void Mesh3D::Draw(matrix_stack& stack) const
{
	for (renderable obj : object)
	{
		obj.bind();
		stack.push();
		stack.mult(obj.transform);
		if (scale != 1.f) stack.mult(glm::scale(glm::mat4(1), glm::vec3(scale)));

		if ((*Mesh3D::_shader).has_uniform("uColor"))
			glUniform4f((*Mesh3D::_shader)["uColor"], obj.mater.base_color_factor[0], obj.mater.base_color_factor[1], obj.mater.base_color_factor[2], obj.mater.base_color_factor[3]);

		if ((*Mesh3D::_shader).has_uniform("uTextureAvailable"))
		{
			glUniform1i((*Mesh3D::_shader)["uTextureAvailable"], obj.mater.use_texture);
			glBindTexture(GL_TEXTURE_2D, obj.mater.base_color_texture);
		}
		glUniformMatrix4fv((*Mesh3D::_shader)["uModel"], 1, GL_FALSE, &stack.m()[0][0]);

		glDrawElements(obj().mode, obj().count, obj().itype, 0);
		stack.pop();
	}
}

bool Mesh3D::CreateRenderable()
{
	// load texture
	for (unsigned int it = 0; it < model.textures.size(); ++it)
	{
		tinygltf::Texture& texture = model.textures[it];
		tinygltf::Sampler sampler = model.samplers[model.textures[it].sampler];
		tinygltf::Image  image = model.images[model.textures[it].source];

		int gl_format;
		switch (image.component)
		{
			case 1: gl_format = GL_ALPHA;	break;
			case 3: gl_format = GL_RGB;		break;
			case 4: gl_format = GL_RGBA;	break;
			default: assert(0);
		}

		// decode the image
		tinygltf::BufferView bufferview = model.bufferViews[image.bufferView];
		tinygltf::Buffer buffer = model.buffers[bufferview.buffer];
		unsigned char* v_ptr = &buffer.data[bufferview.byteOffset];

		int x, y;
		int  channels_in_file;
		stbi_uc* data = stbi_load_from_memory(v_ptr, bufferview.byteLength, &x, &y, &channels_in_file, image.component);

		//			stbi_write_png("read_texture.png", x, y, 4, data, 0);

		id_textures.push_back(0);
		glGenTextures(1, &id_textures.back());

		glBindTexture(GL_TEXTURE_2D, id_textures.back());
		glTexImage2D(GL_TEXTURE_2D, 0, gl_format, image.width, image.height, 0, gl_format, image.pixel_type, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);

		if (sampler.minFilter == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST ||
			sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST ||
			sampler.minFilter == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR ||
			sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR
			)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
	check_gl_errors(QUI);

	const tinygltf::Scene& scene = model.scenes[model.defaultScene];
	for (unsigned int i = 0; i < scene.nodes.size(); i++)
	{
		xassert(scene.nodes[i] >= 0 && scene.nodes[i] < model.nodes.size(), (std::string("Error: invalid model: " + filename)).c_str(), QUI);

		//static glm::mat4 currT(1.f);
		VisitNodes(model.nodes[scene.nodes[i]], glm::mat4(1.f));
	}

	for (unsigned int ir = 0; ir < object.size(); ++ir)
		for (unsigned int ic = 0; ic < 8; ++ic)
			bbox.add(object[ir].transform * glm::vec4(object[ir].bbox.p(ic), 1.0));

	return true;
}

void Mesh3D::VisitNodes(tinygltf::Node& node, glm::mat4 currT)
{
	const std::vector<double>& m = node.matrix;
	glm::mat4 transform(1.f);
	if (!m.empty()) 
		transform = glm::mat4
			(
			m[0],  m[1],  m[2],  m[3],
			m[4],  m[5],  m[6],  m[7],
			m[8],  m[9],  m[10], m[11],
			m[12], m[13], m[14], m[15]
			);
	// GLTF specifica che se 'matrix' è presente, 'translation', 'rotation', 'scale' sono ignorati.
	// Altrimenti, devi costruire local_transform da translation, rotation (quaternion) e scale.
	else if (!node.translation.empty() || !node.rotation.empty() || !node.scale.empty())
	{
		glm::mat4 translation_mat(1.0f);
		if (!node.translation.empty())
			translation_mat = glm::translate(glm::mat4(1.0f), glm::vec3(node.translation[0], node.translation[1], node.translation[2]));

		glm::mat4 rotation_mat(1.0f);
		if (!node.rotation.empty())
		{
			// tinygltf::Node::rotation è un array di 4 double (x, y, z, w)
			glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
			rotation_mat = glm::mat4_cast(q);
		}

		glm::mat4 scale_mat(1.0f);
		if (!node.scale.empty())
			scale_mat = glm::scale(glm::mat4(1.0f), glm::vec3(node.scale[0], node.scale[1], node.scale[2]));

		transform = translation_mat * rotation_mat * scale_mat;
	}

	currT = currT * transform;

	if (node.mesh >= 0 && node.mesh < model.meshes.size()) VisitMesh(model.meshes[node.mesh], currT);

	for (unsigned int i = 0; i < node.children.size(); i++)
	{
		xassert(node.children[i] >= 0 && node.children[i] < model.nodes.size(), std::string("Error: invalid node in " + filename).c_str(), QUI);
		VisitNodes(model.nodes[node.children[i]], currT);
	}
}

void Mesh3D::VisitMesh(tinygltf::Mesh& mesh, glm::mat4 currT)
{
	for (unsigned int i = 0; i < model.bufferViews.size(); i++)
	{
		const tinygltf::BufferView& bufferView = model.bufferViews[i];

		if (bufferView.target == 0) continue;

		const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

		// ebo
	}

	for (unsigned int i = 0; i < mesh.primitives.size(); i++)
	{
		tinygltf::Primitive primitive = mesh.primitives[i];

		object.push_back(renderable());
		renderable& r = object.back();
		r.create();
		r.transform = currT;

		for (auto& attrib : primitive.attributes)
		{
			assert(attrib.second >= 0);
			const tinygltf::Accessor& accessor = model.accessors[attrib.second];

			int n_chan = 1;
			if (accessor.type == TINYGLTF_TYPE_SCALAR) n_chan = 1;
			else if (accessor.type == TINYGLTF_TYPE_VEC2) n_chan = 2;
			else if (accessor.type == TINYGLTF_TYPE_VEC3) n_chan = 3;
			else if (accessor.type == TINYGLTF_TYPE_VEC4) n_chan = 4;
			else assert(0);

			// it->first would be "POSITION", "NORMAL", "TEXCOORD_0", ...
			int attr_index = -1;
			if (attrib.first.compare("POSITION") == 0) attr_index = 0;
			if (attrib.first.compare("COLOR") == 0)    attr_index = 1;
			if (attrib.first.compare("NORMAL") == 0)   attr_index = 2;
			if (attrib.first.compare("TANGENT") == 0)   attr_index = 3;
			if (attrib.first.find("TEXCOORD_") != std::string::npos) {
				std::string n = attrib.first.substr(9, 3);
				attr_index = 4 + atoi(n.c_str());
			}

			if (attr_index != -1)
			{
				int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
				xassert(byteStride != -1, "", QUI);
			}

			n_vert = (int)accessor.count;
			int n_comp = accessor.type; // only consider vec2, vec3 and vec4 (TINYGLTF_TYPE_VEC* )

			size_t buffer = model.bufferViews[accessor.bufferView].buffer;
			size_t bufferviewOffset = model.bufferViews[accessor.bufferView].byteOffset;

			switch (accessor.componentType)
			{
				case TINYGLTF_PARAMETER_TYPE_FLOAT: r.add_vertex_attribute<float>((float*)&model.buffers[buffer].data[bufferviewOffset + accessor.byteOffset], n_comp * n_vert, attr_index, n_comp); break;
				case TINYGLTF_PARAMETER_TYPE_BYTE: r.add_vertex_attribute<char>((char*)&model.buffers[buffer].data[bufferviewOffset + accessor.byteOffset], n_comp * n_vert, attr_index, n_comp); break;
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: r.add_vertex_attribute<unsigned char>((unsigned char*)&model.buffers[buffer].data[bufferviewOffset + accessor.byteOffset], n_comp * n_vert, attr_index, n_comp); break;
			}

			// if the are the position compute the object bounding box
			if (attr_index == 0)
			{
				float* v_ptr = (float*)&model.buffers[buffer].data[bufferviewOffset + accessor.byteOffset];

				for (int iv = 0; iv < n_vert; ++iv)
					r.bbox.add(glm::vec3(*(v_ptr + iv * 3), *(v_ptr + iv * 3 + 1), *(v_ptr + iv * 3 + 2)));
			}
		}
		
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		int mode;
		switch (primitive.mode)
		{
			case(TINYGLTF_MODE_TRIANGLES): mode = GL_TRIANGLES; break;
			case(TINYGLTF_MODE_TRIANGLE_STRIP): mode = GL_TRIANGLE_STRIP; break;
			case(TINYGLTF_MODE_TRIANGLE_FAN): mode = GL_TRIANGLE_FAN; break;
			case(TINYGLTF_MODE_POINTS): mode = GL_POINTS; break;
			case(TINYGLTF_MODE_LINE): mode = GL_LINES; break;
			case(TINYGLTF_MODE_LINE_LOOP): mode = GL_LINE_LOOP; break;
			default: xterminate(std::string("Invalid primitive.mode for " + filename).c_str(), QUI);
		}

		// Compute byteStride from Accessor + BufferView combination.
		int byteStride = indexAccessor.ByteStride(model.bufferViews[indexAccessor.bufferView]);
		xassert(byteStride != -1, "", QUI);

		// one long texture, just a stub implementation
		int buffer = model.bufferViews[indexAccessor.bufferView].buffer;
		size_t bufferviewOffset = model.bufferViews[indexAccessor.bufferView].byteOffset;

		size_t n_tri = indexAccessor.count / 3;

		check_gl_errors(QUI);

		switch (indexAccessor.componentType)
		{
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:     r.add_indices<unsigned char>((unsigned char*)&model.buffers[buffer].data[bufferviewOffset + indexAccessor.byteOffset], (unsigned int)indexAccessor.count, GL_TRIANGLES); break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:    r.add_indices<unsigned short>((unsigned short*)&model.buffers[buffer].data[bufferviewOffset + indexAccessor.byteOffset], (unsigned int)indexAccessor.count, GL_TRIANGLES); break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:      r.add_indices<unsigned int>((unsigned int*)&model.buffers[buffer].data[bufferviewOffset + indexAccessor.byteOffset], (unsigned int)indexAccessor.count, GL_TRIANGLES); break;
		}

		check_gl_errors(QUI);

		// setup the material
		tinygltf::Material mat = model.materials[primitive.material];
		int index;

		memcpy_s(r.mater.base_color_factor, sizeof(double) * 4, &mat.pbrMetallicRoughness.baseColorFactor[0], sizeof(double) * 4);

		index = mat.pbrMetallicRoughness.baseColorTexture.index;
		r.mater.base_color_texture = (index != -1) ? this->id_textures[index] : this->id_textures.empty() ? -1 : this->id_textures[0];
		if (index == -1) r.mater.use_texture = false;

		index = mat.normalTexture.index;
		r.mater.normal_texture = (index != -1) ? this->id_textures[index] : -1;

		index = mat.emissiveTexture.index;
		r.mater.emissive_texture = (index != -1) ? this->id_textures[index] : -1;
	}
}

std::string Mesh3D::GetFilePathExtension(const std::string& FileName)
{
	if (FileName.find_last_of(".") != std::string::npos) return FileName.substr(FileName.find_last_of(".") + 1);
	else return "";
}