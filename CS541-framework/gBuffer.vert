/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 400



uniform vec3 lightPos;
uniform mat4 WorldView;
uniform mat4 WorldInverse;
uniform mat4 WorldProj;
uniform mat4 ModelTr;
uniform mat4 NormalTr;


in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

out vec3 normalVec, lightVec, eyeVec;
out vec4 worldPos;
//out vec4 shadowCoord;
out vec2 texCoord;
out vec4 tangent;




void main()
{      
    gl_Position = WorldProj*WorldView*ModelTr*vertex;
    
    //worldPos = (ModelTr*vertex).xyz;
	worldPos.xyz = (ModelTr*vertex).xyz;
	tangent = ModelTr * vec4(vertexTangent.xyz,0);
    normalVec = (vertexNormal*mat3(NormalTr)); 
    lightVec = lightPos - worldPos.xyz;
	//eyeVec = (WorldInverse * vec4(0.f, 0.f, 0.f, 1.f)).xyz-worldPos.xyz;
    
	eyeVec = (WorldInverse * vec4(0.f, 0.f, 0.f, 1.f)).xyz-worldPos.xyz;
	texCoord = vertexTexture; 

	//shadowCoord = ShadowMatrix*ModelTr*vertex;
}
