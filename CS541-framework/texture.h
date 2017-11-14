///////////////////////////////////////////////////////////////////////
// A slight encapsulation of an OpenGL texture. This contains a method
// to read an image file into a texture, and methods to bind a texture
// to a shader for use, and unbind when done.
////////////////////////////////////////////////////////////////////////
#include <vector>
#ifndef _TEXTURE_
#define _TEXTURE_


struct basicVec3
{
	float x, y, z;
};

class Texture
{
 public:
    unsigned int textureId;
	int HDRwidth, HDRheight;
	std::vector<basicVec3> HDRvals;
	//Sean added this
	Texture() : textureId(0), HDRwidth(-1), HDRheight(-1), HDRvals() {}
	Texture(int width, int height);


    Texture(const std::string &filename);
    void Bind(const int unit);
    void Unbind();


	void MakeHDRTexture(const std::string& filename);


};






#endif
