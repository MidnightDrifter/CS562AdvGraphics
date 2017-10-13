/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 330

// These definitions agree with the ObjectIds enum in scene.h
const int     nullId	= 0;
const int     skyId	= 1;
const int     seaId	= 2;
const int     groundId	= 3;
const int     wallId	= 4;
const int     boxId	= 5;
const int     frameId	= 6;
const int     lPicId	= 7;
const int     rPicId	= 8;
const int     teapotId	= 9;
const int     spheresId	= 10;
const float PI = 3.1415926535897932384626433832795;

const float EPSILON = 0.01;


in vec3 normalVec, lightVec, eyeVec;
in vec4 pos;
//in vec2 texCoord;

//uniform int objectId;
//uniform vec3 diffuse;
uniform mat4 WorldInverse;
uniform vec3 lightPos;
uniform sampler2D gBuffer0;  //WorldPos.xyz, worldPosDepth
uniform sampler2D gBuffer1;  //specular.xyz, shininess
uniform sampler2D gBuffer2;  //diffuse.xyz
uniform sampler2D gBuffer3;  //normalVec.xyz
uniform vec3 localLightBrightness;
uniform float radius;
//uniform vec3 ObjectCenter;
uniform float radiusSquared;
//uniform vec4 ObjectCenter;


in vec4 center;

uniform int width;
uniform int height;
vec3 BRDF(vec3 nVec, vec3 lVec, vec3 eVec, float shiny, vec3 spec, vec3 dif)
{

	vec3 N = normalize(nVec);
	vec3 L = normalize(lVec);
	vec3 V = normalize(eVec);
	vec3 H = normalize(L+V);
	
	float alpha = shiny;   //pow(8192, shiny);
	//float LN = max(0.f, dot(L,N));
	float LH = max(0.f, dot(L,H));
	float NH = max(0.f,dot(N,H));

	float gValue = 1 / (pow(LH,2)*4);   //Raised to power of 2, no need to care about negative vals -- maybe div. by 0 though
	float dValue = ((2+alpha)/(PI*2))*(pow(NH,alpha));
	vec3 fValue = spec + ((1-spec)*(pow((1-LH),5)));

	return (dif/PI)+(gValue*dValue*fValue);
	

}

vec3 BRDFSpec(vec3 nVec, vec3 lVec, vec3 eVec, float shiny, vec3 spec, vec3 dif)
{
	vec3 N = normalize(nVec);
	vec3 L = normalize(lVec);
	vec3 V = normalize(eVec);
	vec3 H = normalize(L+V);
	
	float alpha = shiny;   //pow(8192, shiny);
	//float LN = max(0.f, dot(L,N));
	float LH = max(0.f, dot(L,H));
	float NH = max(0.f,dot(N,H));

	float gValue = 1 / (pow(LH,2)*4);   //Raised to power of 2, no need to care about negative vals -- maybe div. by 0 though
	float dValue = ((2+alpha)/(PI*2))*(pow(NH,alpha));
	vec3 fValue = spec + ((1-spec)*(pow((1-LH),5)));

	return (gValue*dValue*fValue);
}
/*
vec3 BRDF()
{
	return BRDF(normalVec, lightVec, eyeVec, shininess, specular, diffuse);
}

vec3 BRDFSpec()
{
	return BRDFSpec(normalVec, lightVec, eyeVec, shininess, specular, diffuse);
}
*/
float random(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}


void main()
{

	vec2 myPixelCoordinate = vec2(gl_FragCoord.x/width, gl_FragCoord.y/height);  //I forget the call for this fug
	
	vec4 worldPos = texture2D(gBuffer0,myPixelCoordinate).xyzw;
	float  worldPosDepth = texture2D(gBuffer0,myPixelCoordinate).w;
	
	vec3 specular = texture2D(gBuffer1, myPixelCoordinate).xyz;
	float shininess = texture2D(gBuffer1, myPixelCoordinate).w;

	vec3 diffuse = texture2D(gBuffer2, myPixelCoordinate).xyz;
	vec3 L = normalize(lightVec);
	vec3 N = texture2D(gBuffer3,myPixelCoordinate).xyz;
	float LN = max(dot(L,N),0.0f);
	float radiusSquared = radius*radius;

	//if pixel out of light's range:  gl_FragColor.xyz = vec3(0,0,0);
	//else gl_FragColor.xyz = LN* *BRDF(N,lightVec,eyeVec,shininess,specular,diffuse );
	
	//vec3 lightDistance = pos.xyz-center.xyz;
	//vec3 lightDistance = worldPos.xyz - ObjectCenter.xyz;
	vec3 lightDistance = center.xyz - worldPos.xyz;  //worldPos.xyz - center.xyz;
	gl_FragColor.a = 0.5;

	vec3 lD = normalize(lightDistance);
	//vec3 EV = normalize()
	LN = max(dot(lD,N),0.f);


	 if(radiusSquared -dot(lightDistance,lightDistance) <= EPSILON)
	{
	
//	gl_FragColor.xyz=vec3(0.5,0,0);
gl_FragColor.xyz = LN* localLightBrightness *BRDF(N,lD,eyeVec,shininess,specular,diffuse );
	//gl_FragColor.xyz = vec3(0,1,0);
	}

	else
	{
	//gl_FragColor.xyz = LN* localLightBrightness *BRDF(N,lightVec,eyeVec,shininess,specular,diffuse );
	gl_FragColor.xyz = vec3(0,0,0);
	
	}

}
