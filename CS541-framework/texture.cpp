///////////////////////////////////////////////////////////////////////
// A slight encapsulation of an OpenGL texture. This contains a method
// to read an image file into a texture, and methods to bind a texture
// to a shader for use, and unbind when done.
////////////////////////////////////////////////////////////////////////

#include "math.h"
#include <fstream>
#include <stdlib.h>

#include "texture.h"
#include "rgbe.h"
#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;
#include <freeglut.h>
#include <glu.h>   


#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

#define CHECKERROR\
 {GLenum err = glGetError();\
  if (err != GL_NO_ERROR) {\
fprintf(stdout, "OpenGL error (at line %d): %s\n", __LINE__, gluErrorString(err));}}


Texture::Texture(int width, int height) : textureId(0), HDRwidth(-1), HDRheight(-1)
{
	

	glGenTextures(1, &textureId);   // Get an integer id for thi texture from OpenGL
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 100);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}



Texture::Texture(const std::string &path) : textureId(0)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, n;
    unsigned char* image = stbi_load(path.c_str(), &width, &height, &n, 4);
    if (!image) {
        printf("\nRead error on file %s:\n  %s\n\n", path.c_str(), stbi_failure_reason());
        exit(-1); }

    // Here we create MIPMAP and set some useful modes for the texture
    glGenTextures(1, &textureId);   // Get an integer id for thi texture from OpenGL
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 100);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);  
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);
}

void Texture::Bind(const int unit)
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void Texture::Unbind()
{  
    glBindTexture(GL_TEXTURE_2D, 0);
}


void Texture::MakeHDRTexture(const std::string& filename) 
{

	int width, height;// = nullptr;
	//int* height = nullptr;
	rgbe_header_info headerInfo;   // = nullptr;
	char errBuff[100] = { 0 };

	FILE* fp(fopen(filename.c_str(), "rb"));

	int errCode = RGBE_ReadHeader(fp, &width, &height, &headerInfo, errBuff);

	std::vector<float> hdrInputVals(3*(width)*(height));

	HDRwidth = width;
	HDRheight = height;

//	fopen(filename.c_str(),fp

	if (errCode == RGBE_RETURN_FAILURE)
	{
		//An error has occurred, exit!
		exit(-2);
	}


	errCode = RGBE_ReadPixels_RLE(fp, hdrInputVals.data(), width, height, errBuff);



	if (-1 == fclose(fp))
	{
		exit(-4);
	}



	if (errCode == RGBE_RETURN_FAILURE)
	{
		exit(-3);
	}

	//std::vector<char> image(hdrInputVals.size());

	glGenTextures(1, &textureId);   // Get an integer id for thi texture from OpenGL
	glBindTexture(GL_TEXTURE_2D, textureId);
	CHECKERROR;
	glTexImage2D(GL_TEXTURE_2D, 0, (int)GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, hdrInputVals.data());
	CHECKERROR;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 100);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	

//NO idea if this works at all



}