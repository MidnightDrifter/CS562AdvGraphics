/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr, NormalTr;

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

out vec3 normalVec, lightVec, eyeVec;
out vec4 pos, center;
//out vec2 texCoord;
uniform vec3 lightPos;
uniform vec3 ObjectCenter;
void main()
{     // gl_Position = vertex;
   
   gl_Position = WorldProj*WorldView*ModelTr*vertex;
    pos = gl_Position;
    vec3 worldPos = (ModelTr*vertex).xyz;

	center = ModelTr*vec4(ObjectCenter,1);

    normalVec = vertexNormal*mat3(NormalTr); 
    lightVec = lightPos - worldPos;
		eyeVec = (WorldInverse * vec4(0.f, 0.f, 0.f, 1.f)).xyz-worldPos;
   // texCoord = vertexTexture; 


}
