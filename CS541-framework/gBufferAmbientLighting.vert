/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

//uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr, NormalTr;

//for FSQ
//const vec2 quadVertices[4] = { vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0) };
//  To draw FSQ, just:    
//gl_Position = vec4(quadVertices[gl_VertexID], 0.0, 1.0);

uniform mat4 WorldInverse, ModelTr;
in vec4 vertex;
//in vec3 vertexNormal;
//in vec2 vertexTexture;
in vec3 vertexTangent;
out vec4 tangent;
//out vec3 normalVec, lightVec;
//out vec2 texCoord;
//uniform vec3 lightPos;
out vec3 eyePos;
void main()
{      

gl_Position =vertex;

//gl_Position = vec4(quadVertices[gl_VertexID], 0.0, 1.0);


    //gl_Position = WorldProj*WorldView*ModelTr*vertex;
    
    vec3 worldPos = (ModelTr*vertex).xyz;
	eyePos = (WorldInverse * vec4(0.f, 0.f, 0.f, 1.f)).xyz-worldPos;
    //normalVec = vertexNormal*mat3(NormalTr); 
    //lightVec = lightPos - worldPos;
	tangent = ModelTr * vec4(vertexTangent.xyz,0);
    //texCoord = vertexTexture; 
}
