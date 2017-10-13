/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr, NormalTr, ShadowMatrix;

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;

out vec3 normalVec, lightVec, eyeVec, reflectVec, transformLightVec, transformEyeVec;
out vec4 shadowCoord, tangent;
out vec2 texCoord;
uniform vec3 lightPos;



void main()
{     
	vec3 centerOfScene = vec3(0.f, 0.f,4.5f);  //Teapot is in the center of the scene at ~ origin
	
	vec3 worldPos = (ModelTr*vertex).xyz;// - vec3(0,12,0);

	vec3 RG =  worldPos - centerOfScene;
    reflectVec = normalize(RG);
	float d = 1+reflectVec.z;
	float RMagnitude = length(RG);
	gl_Position = vec4(reflectVec.x/d, reflectVec.y/d,((reflectVec.z*RMagnitude)/100.f)-0.9999f,1); //WorldProj*WorldView*ModelTr*vertex;
    //gl_Position = vec4(reflectVec.x/d, reflectVec.y/d, -RMagnitude / (RMagnitude+1)/165.f,1);
    

    normalVec = (vertexNormal*mat3(NormalTr)); 
    lightVec = lightPos - worldPos;
	eyeVec = (WorldInverse * vec4(0.f, 0.f, 0.f, 1.f)).xyz-worldPos;

    transformLightVec = lightPos-centerOfScene;
	transformEyeVec = (WorldInverse * vec4(0.f, 0.f,0.f, 1.f)).xyz - centerOfScene;
	
	texCoord = vertexTexture; 
		tangent = ModelTr * vec4(vertexTangent.xyz,0);
	shadowCoord = ShadowMatrix*ModelTr*vertex;
}
