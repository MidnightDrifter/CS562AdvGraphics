/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 ShadowView, ShadowProj, ModelTr;

in vec4 vertex;
//in vec3 vertexNormal;
//in vec2 vertexTexture;
//in vec3 vertexTangent;

//out vec3 normalVec, lightVec;
//out vec2 texCoord;
//uniform vec3 lightPos;
out vec4 pos;
void main()
{       pos = ShadowProj*ShadowView*ModelTr*vertex;
    gl_Position = pos;
   
    //vec3 worldPos = (ModelTr*vertex).xyz;

    //normalVec = vertexNormal*mat3(NormalTr); 
    //lightVec = lightPos - worldPos;

    //texCoord = vertexTexture; 
}
