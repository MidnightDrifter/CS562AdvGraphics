/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 400

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


float SpiralPseudoRandom(ivec2 in)
{
 return (30 * in.x ^ in.y) + 10*in.x*in.y;

}


uniform float rangeOfInfluence;  //Ambient occlusion R
const float aOC = 0.1*rangeOfInfluence;
const float aOCSquared = aOC*aOC;
const float ambientOcclusionThreshold = 0.001;


uniform sampler2D gBuffer0;  //WorldPos.xyz, worldPosDepth
uniform sampler2D gBuffer3;  //normalVec.xyz
uniform int NumSamples;
//uniform float 

void main()
{/*
    vec3 N = normalize(normalVec);
    vec3 L = normalize(lightVec);

    vec3 Kd = diffuse;   
    
    if (objectId==groundId || objectId==seaId) {
        ivec2 uv = ivec2(floor(200.0*texCoord));
        if ((uv[0]+uv[1])%2==0)
            Kd *= 0.9; }
    
    gl_FragColor.xyz = vec3(0.5,0.5,0.5)*Kd + Kd*max(dot(L,N),0.0);
	*/

		vec2 myPixelCoordinate = vec2(gl_FragCoord.x/ width, gl_FragCoord.y/height);  

	vec3 worldPos = texture2D(gBuffer0,myPixelCoordinate).xyz;
	float  worldPosDepth = texture2D(gBuffer0,myPixelCoordinate).w;
		vec3 N = normalize(texture2D(gBuffer3,myPixelCoordinate).xyz);

		float out =0;
	
	for(int i=0;i<NumSamples;i++)
	{
	float a = (i+0.5)/NumSamples;
	float h = (a*R) / worldPosDepth;
	float theta = (2*PI*a) *(7*NumSamples/9) * SprialPseudoRandom(gl_FragCoord.xy);


	vec2 otherPixelCoord = myPixelCoordinate + h*(cosf(theta), sinf(theta));
	vec3 otherPos = texture2D(gBuffer0,otherPixelCoord).xyz;
	float otherDepth = texture2D(gBuffer0,otherPixelCoord).w;
	//vec3 otherNorm = 
	vec3 wI = otherPos - worldPos;

	if(rangeOfInfluence < length(wI))
	{
	
	out += (max(0,dot(N,wI)- ambientOcclusionThreshold*otherDepth)) / max(aOCSquared,dot(wI,wI));

	}


	
	}


	out *= (2*PI*aOC)/NumSamples;

	gl_FragData[0] = vec4(out);


}
