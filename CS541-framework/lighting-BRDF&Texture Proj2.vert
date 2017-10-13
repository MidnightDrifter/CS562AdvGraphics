/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

//for FSQ
const vec2 quadVertices[4] = { vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0) };
//  To draw FSQ, just:    
//gl_Position = vec4(quadVertices[gl_VertexID], 0.0, 1.0);
uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr, NormalTr, ShadowMatrix;

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

out vec3 normalVec, lightVec, eyeVec;
out vec4 shadowCoord;
out vec2 texCoord;
out vec4 tangent;
uniform vec3 lightPos;



void main()
{      
    gl_Position = WorldProj*WorldView*ModelTr*vertex;
    



    vec3 worldPos = (ModelTr*vertex).xyz;
	tangent = ModelTr * vec4(vertexTangent.xyz,0);
    normalVec = (vertexNormal*mat3(NormalTr)); 
    lightVec = lightPos - worldPos;
	eyeVec = (WorldInverse * vec4(0.f, 0.f, 0.f, 1.f)).xyz-worldPos;
    texCoord = vertexTexture; 

	shadowCoord = ShadowMatrix*ModelTr*vertex;
}
