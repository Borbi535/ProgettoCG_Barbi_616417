#include<shadowmap.hpp>



frame_buffer_object::frame_buffer_object(int w_, int h_, bool _use_texture_for_depth)
{
	w = w_;
	h = h_;
	glGenFramebuffers(1, &this->id_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, this->id_fbo);

	/* texture for color attachment*/
	glGenTextures(1, &this->id_tex);
	glBindTexture(GL_TEXTURE_2D, this->id_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->id_tex, 0); // texture del framebuffer

	/* texture for color attachment 1*/
	glGenTextures(1, &this->id_tex1);
	glBindTexture(GL_TEXTURE_2D, this->id_tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->id_tex1, 0);

	// Se il tuo shader per la depth map scrive solo a GL_COLOR_ATTACHMENT0:
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers); // 1 è il numero di attacchi attivi

	// Se il tuo shader dovesse scrivere a entrambi (es. per future estensioni o se id_tex1 viene usato in qualche modo):
	// GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	// glDrawBuffers(2, drawBuffers); // 2 è il numero di attacchi attivi

	check_gl_errors(QUI, true);
	use_texture_for_depth = _use_texture_for_depth;
	if (_use_texture_for_depth)
	{
		/* texture for depth  attachment*/
		glGenTextures(1, &this->id_depth);
		glBindTexture(GL_TEXTURE_2D, this->id_depth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->id_depth, 0);
	}
	else
	{	/* render buffer for depth  attachment*/
		glGenRenderbuffers(1, &this->id_depth);
		glBindRenderbuffer(GL_RENDERBUFFER, this->id_depth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->id_depth);
	}
	check_gl_errors(__LINE__, __FILE__, true);
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	check(status);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

frame_buffer_object::~frame_buffer_object()
{
	glDeleteFramebuffers(1, &this->id_fbo);

	if (use_texture_for_depth) glDeleteTextures(1, &this->id_depth);
	else glDeleteRenderbuffers(1, &this->id_depth);
}

void frame_buffer_object::check(int fboStatus)
{
	switch (fboStatus)
	{
		case GL_FRAMEBUFFER_COMPLETE:break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:          std::cout << "FBO Incomplete: Attachment\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:  std::cout << "FBO Incomplete: Missing Attachment\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:      std::cout << "FBO Incomplete: Dimensions\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:         std::cout << "FBO Incomplete: Formats\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         std::cout << "FBO Incomplete: Draw Buffer\n"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         std::cout << "FBO Incomplete: Read Buffer\n"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED:                    std::cout << "FBO Unsupported\n"; break;
		default:                                            std::cout << "Undefined FBO error\n"; break;
	}
}

/***************************************************************************************************/

shadowmap_texture_array::shadowmap_texture_array(int size_x, int size_y)
{
	this->size_x = size_x;
	this->size_y = size_y;

	glGenTextures(1, &id_tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, id_tex);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RG32F, size_x, size_y, MAX_SHADOWMAP_TEXARRAY_LAYERS);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // Per l'indice di strato

	//float border_color[] = { 1.f, 1.f, 1.f, 1.f };
	//glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_color);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	check_gl_errors(QUI);
}

void shadowmap_texture_array::update(std::vector<std::shared_ptr<frame_buffer_object>> fbos)
{
	xassert(fbos.size() < MAX_SHADOWMAP_TEXARRAY_LAYERS, "fbos vector size exceeds the max number of layer possible. ", QUI);

	glBindTexture(GL_TEXTURE_2D_ARRAY, id_tex);

	for (int i = 0; i < fbos.size(); ++i)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos[i]->id_fbo);

		glCopyTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,             // Livello MIP
			0, 0, i,       // Offset X, Y, Z (Z è l'indice dello strato di destinazione)
			0, 0,          // Offset X, Y dell'origine della copia nel GL_READ_FRAMEBUFFER
			size_x,        // Larghezza da copiare
			size_y);       // Altezza da copiare
		check_gl_errors(QUI);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}
