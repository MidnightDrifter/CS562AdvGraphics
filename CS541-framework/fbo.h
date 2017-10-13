///////////////////////////////////////////////////////////////////////
// A slight encapsulation of a Frame Buffer Object (i'e' Render
// Target) and its associated texture.  When the FBO is "Bound", the
// output of the graphics pipeline is captured into the texture.  When
// it is "Unbound", the texture is available for use as any normal
// texture.
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////

class FBO {
public:
    unsigned int fbo;

    unsigned int texture;
	unsigned int renderTargets[4];  //ONLY FOR USE WITH G BUFFER!!!!!
    int width, height;  // Size of the texture.
	

	//GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	//glDrawBuffers(GBUFFER_NUM_TEXTURES, DrawBuffers);


    void CreateFBO(const int w, const int h);
	void CreateGBuffer(const int w, const int h);
    void Bind();
    void Unbind();
};
