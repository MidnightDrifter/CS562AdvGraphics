﻿/////////////////////////////////////////////////////////////////////////
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
//in vec3 normalVec, lightVec;
//in vec2 texCoord;

//uniform int objectId;
//uniform vec3 diffuse;

uniform sampler2D gBuffer0;  //WorldPos.xyz, worldPosDepth
uniform sampler2D gBuffer1;  //specular.xyz, shininess
uniform sampler2D gBuffer2;  //diffuse.xyz
uniform sampler2D gBuffer3;  //normalVec.xyz


uniform sampler2D skydomeTexture;
uniform sampler2D irradianceMap;

//uniform sampler2D 

uniform HammersleyBlock {
int HamN;
float hammersley[2*N]; };




uniform vec3 ambient;

uniform int width;
uniform int height;

uniform mat4 WorldInverse;

uniform float exposure;
uniform float contrast;





/*

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


*/

float gValue(vec3 lVec, vec3 eVec, vec3 nVec)
{
//vec3 N = normalize(nVec);
vec3 L = normalize(lVec);
vec3 V = normalize(eVec);
vec3 H = normalize(lVec+eVec);

return 1/(4*pow(dot(L,H),2));

}


float dValue(vec3 lVec, vec3 eVec, vec3 nVec, float alpha)
{

vec3 N = normalize(nVec);
vec3 L = normalize(lVec);
vec3 V = normalize(eVec);
vec3 H = normalize(lVec+eVec);


return ((2+alpha)/(PI*2))*(pow(dot(N,H),alpha));


}


vec3 fValue(vec3 lVec, vec3 eVec, vec3 nVec, vec3 spec)
{

vec3 N = normalize(nVec);
vec3 L = normalize(lVec);
vec3 V = normalize(eVec);
vec3 H = normalize(lVec+eVec);


return spec + ((1-spec)*(pow((1-dot(L,H)),5)));


}


void main()
{
   // vec3 N = normalize(normalVec);
  //  vec3 L = normalize(lightVec);

  //  vec3 Kd = diffuse;   
    
  //  if (objectId==groundId || objectId==seaId) {
  //      ivec2 uv = ivec2(floor(200.0*texCoord));
 //       if ((uv[0]+uv[1])%2==0)
  //          Kd *= 0.9; }
    
  //  gl_FragColor.xyz = vec3(0.5,0.5,0.5)*Kd + Kd*max(dot(L,N),0.0);


 	vec2 myPixelCoordinate = vec2(gl_FragCoord.x/width, gl_FragCoord.y/height);  //I forget the call for this fug
	
//	vec3 worldPos = texture2D(gBuffer0,myPixelCoordinate).xyz;
//	float  worldPosDepth = texture2D(gBuffer0,myPixelCoordinate).w;
	
	vec3 specular = texture2D(gBuffer1, myPixelCoordinate).xyz;
	float shininess = texture2D(gBuffer1, myPixelCoordinate).w;

  	vec3 diffuse = texture2D(gBuffer2, myPixelCoordinate).xyz;
	//vec3 L = normalize(lightVec);
	vec3 N = normalize(texture2D(gBuffer3,myPixelCoordinate).xyz);
	//float LN = max(dot(L,N),0.0f);




//	gl_FragColor.xyz = N;
//	gl_FragColor.xyz = diffuse;
//	gl_FragColor.xyz = specular;
//  gl_FragColor.xyz = worldPos.xyz;





if(texture2D(gBuffer3,myPixelCoordinate).w == skyId)
{
//Lin. Color Space & Tone Mapping
vec3 outColor = vec3(0,0,0);
vec3 inColor = texture2D(skydomeTexture, skyTexCoord).xyz;

inColor = pow(inColor, vec3(2.2));












	vec2 myPixelCoordinate = vec2(gl_FragCoord.x/ width, gl_FragCoord.y/height);  

	vec3 worldPos = texture2D(gBuffer0,myPixelCoordinate).xyz;
		
		vec3 V = normalize((WorldInverse * vec4(0.f, 0.f, 0.f, 1.f)).xyz-worldPos);



		vec3 R = 2*dot(N,V)*N - V;	

		vec3 A = normalize(cross(vec3(0,0,1), R));
		vec3 B = normalize(cross(R,A));
		vec3 L, lightIntensity,wK, diffuseApprox;
		vec3 monteCarloSum = vec3(0,0,0);
		float level;

		//float monteCarloSum=0;

		//vec3 fValue = spec + ((1-spec)*(pow((1-LH),5)));

		//float gValue = 1 / (pow(LH,2)*4);   //Raised to power of 2, no need to care about negative vals -- maybe div. by 0 though


		for(int i =0; i<2*HamN;i+=2)
		{
		vec2 randTexCoord = vec2(hammersley[i], hammersley[i+1]);
		randTexCoord.y = (acos(pow(randTexCoord.y, (1/(shininess+1)))))/PI;
		
		L = vec3( cos(2*PI*(0.5-randTexCoord.x))*sin(PI*randTexCoord.y),  sin(2*PI*(0.5-randTexCoord.x))*sin(PI*randTexCoord.y),  cos(PI*randTexCoord.y)	);
		wK = normalize(L.x * A + L.y * B + L.z * R);

		level = (  (0.5)* log2(width*height/HamN)  ) - ( 0.5* log2(dValue(wK,V,N,shininess)/4)   ) ;  //Check this--might use L instead of wK
		lightIntensity = textureLod(skydomeTexture, skyTexCoord,level).xyz;

		monteCarloSum += (gValue(wK,V,N) * fValue(wK,V,N,specular)*lightIntensity*max(0,dot(wK,N)));

		}



		monteCarloSum /= HamN;   //Specular bit!!

		//Diffuse bit--just read from irradiance map
		vec2 irradianceTexCoord = (0.5−(atan(N.y ,N.x)/(2*PI)), acos(N.z)/PI );
		diffuseApprox = (diffuse/PI) * texture(irradianceMap, irradianceTexCoord).xyz;



		outColor = diffuseApprox + monteCarloSum;
		



		
vec3 D = -1*V;
//vec3 D = V;
//vec2 skyTexCoord= vec2(0.5f - atan(D.y,D.x), -acos(D.z)/PI);  //Flip the acos to flip the skydome


vec2 skyTexCoord= vec2(0.5f - atan(D.y,D.x)/(2*PI), -acos(D.z)/PI);  //Flip this to flip skysphere
vec3 skyColor = texture(skydomeTexture,skyTexCoord).xyz;

//skyColor = vec3(1.f,0.f,0.f);

gl_FragColor.xyz = skyColor; //  Check this  ???



//Lin. Color Space & Tone Mapping
outColor = ((exposure*outColor)/((exposure*outColor) + vec3(1,1,1)));
outColor = pow(outColor, vec3(contrast/2.2))



return;








}


  gl_FragColor.xyz = ambient;



 // gl_FragColor.xyz = N;
}
