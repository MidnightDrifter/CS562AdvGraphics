//////////////////////////////////////////////////////////////////////
// Defines and draws a scene.  There are two main procedures here:
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////

#include "math.h"
#include <fstream>
#include <stdlib.h>

//#include <freeglut.h>



#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>



using namespace gl;

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE

#include <glm/glm.hpp>
#include <glu.h>      
using namespace glm;

#include "shader.h"
#include "shapes.h"
#include "scene.h"
#include "transform.h"
const float PI = 3.14159f;


#define CHECKERROR\
 {GLenum err = glGetError();\
  if (err != GL_NO_ERROR) {\
fprintf(stderr, "OpenGL error (at line %d) in Object.cpp: %s\n", __LINE__, gluErrorString(err)); \
fprintf(stdout, "OpenGL error (at line %d) in Object.cpp: %s\n ", __LINE__, gluErrorString(err));}}




Object::Object(Shape* _shape, const int _objectId,
               const vec3 _diffuseColor, const vec3 _specularColor, const float _shininess)
    : shape(_shape), objectId(_objectId), textureId(0), normalId(0),
      diffuseColor(_diffuseColor), specularColor(_specularColor), shininess(_shininess)
{}

void Object::SetTexture(Texture* _texture, const MAT4 _textr, Texture* _normal)
{
    textureId = _texture?_texture->textureId:0;
    normalId = _normal?_normal->textureId:0;
    texTr = _textr;
}

void Object::Draw(ShaderProgram* program, MAT4& objectTr)
{
	vec3 lightColor(PI , PI , PI );
	vec3 ambientColor(0.2f*PI, PI*0.2f, PI* 0.2f);


    int loc = glGetUniformLocation(program->programId, "diffuse");
    glUniform3fv(loc, 1, &diffuseColor[0]);

    loc = glGetUniformLocation(program->programId, "specular");
    glUniform3fv(loc, 1, &specularColor[0]);

    loc = glGetUniformLocation(program->programId, "shininess");
    glUniform1f(loc, shininess);

    loc = glGetUniformLocation(program->programId, "objectId");
    glUniform1i(loc, objectId);

	loc = glGetUniformLocation(program->programId, "Light");
	glUniform3fv(loc, 1, &(lightColor[0]));
	loc = glGetUniformLocation(program->programId, "Ambient");
	glUniform3fv(loc, 1, &(ambientColor[0]));

    MAT4 inv;
    invert(&objectTr, &inv);

    loc = glGetUniformLocation(program->programId, "ModelTr");
    glUniformMatrix4fv(loc, 1, GL_TRUE, objectTr.Pntr());
    
    loc = glGetUniformLocation(program->programId, "NormalTr");
    glUniformMatrix4fv(loc, 1, GL_TRUE, inv.Pntr());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    loc = glGetUniformLocation(program->programId, "surfaceTexture");
    glUniform1i(loc, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalId);
    loc = glGetUniformLocation(program->programId, "surfaceNormal");
    glUniform1i(loc, 1);

    loc = glGetUniformLocation(program->programId, "texTr");
    glUniformMatrix4fv(loc, 1, GL_TRUE, texTr.Pntr());

    // Draw this object's triangle
	if (shape) {
		shape->DrawVAO();
		loc = glGetUniformLocation(program->programId, "ObjectCenter");
		glUniform3fv(loc, 1, &(shape->center[0]));
	}
    // Recursively draw each sub-objects, each with its own transformation.
    for (int i=0;  i<instances.size();  i++) {
        MAT4 itr = objectTr*instances[i].second*animTr;
        instances[i].first->Draw(program, itr); }
}


void Object::DrawLights(ShaderProgram* program, MAT4& objectTr)
{
	CHECKERROR;
	vec3 lightColor(PI, PI, PI);
	vec3 ambientColor(0.2f*PI, PI*0.2f, PI* 0.2f);
	vec4 zeroes(0, 0, 0,1);
	int loc;

	// Draw this object's triangle

//	MAT4 inv;
//	invert(&objectTr, &inv);

	loc = glGetUniformLocation(program->programId, "ModelTr");
	glUniformMatrix4fv(loc, 1, GL_TRUE, objectTr.Pntr());
	CHECKERROR;

	if (NULL != shape) {
		
		shape->DrawVAO();
		CHECKERROR;
		loc = glGetUniformLocation(program->programId, "ObjectCenter");
		CHECKERROR;
		glUniform3fv(loc, 1, &(shape->center[0]));
		CHECKERROR;
		
		//	printf("Center is:  (%f, %f, %f)\n", shape->center[0], shape->center[1], shape->center[2]);
	}
	// Recursively draw each sub-objects, each with its own transformation.
	CHECKERROR;
	for (int i = 0; i<instances.size(); i++) {
		MAT4 itr = objectTr*instances[i].second*animTr;
		instances[i].first->DrawLights(program, itr);
		CHECKERROR;
	}
}
