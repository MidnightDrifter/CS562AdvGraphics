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
//in vec3 normalVec, lightVec;
//in vec2 texCoord;

//uniform int objectId;
//uniform vec3 diffuse;

uniform sampler2D gBuffer0;  //WorldPos.xyz, worldPosDepth
uniform sampler2D gBuffer1;  //specular.xyz, shininess
uniform sampler2D gBuffer2;  //diffuse.xyz
uniform sampler2D gBuffer3;  //normalVec.xyz


uniform sampler2D skydomeTexture;

uniform vec3 ambient;

uniform int width;
uniform int height;

uniform mat4 WorldInverse;

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
	
//	vec3 specular = texture2D(gBuffer1, myPixelCoordinate).xyz;
//	float shininess = texture2D(gBuffer1, myPixelCoordinate).w;

//	vec3 diffuse = texture2D(gBuffer2, myPixelCoordinate).xyz;
	//vec3 L = normalize(lightVec);
	//vec3 N = normalize(texture2D(gBuffer3,myPixelCoordinate).xyz);
	//float LN = max(dot(L,N),0.0f);




//	gl_FragColor.xyz = N;
//	gl_FragColor.xyz = diffuse;
//	gl_FragColor.xyz = specular;
//  gl_FragColor.xyz = worldPos.xyz;





if(texture2D(gBuffer3,myPixelCoordinate).w == skyId)
{

	vec2 myPixelCoordinate = vec2(gl_FragCoord.x/ width, gl_FragCoord.y/height);  

	vec3 worldPos = texture2D(gBuffer0,myPixelCoordinate).xyz;
		
		vec3 V = normalize((WorldInverse * vec4(0.f, 0.f, 0.f, 1.f)).xyz-worldPos);




		
vec3 D = -1*V;
//vec3 D = V;
//vec2 skyTexCoord= vec2(0.5f - atan(D.y,D.x), -acos(D.z)/PI);  //Flip the acos to flip the skydome


vec2 skyTexCoord= vec2(0.5f - atan(D.y,D.x)/(2*PI), -acos(D.z)/PI);  //Flip this to flip skysphere
vec3 skyColor = texture(skydomeTexture,skyTexCoord).xyz;

//skyColor = vec3(1.f,0.f,0.f);

gl_FragColor.xyz = skyColor; //  Check this  ???
return;

}


  gl_FragColor.xyz = ambient;



 // gl_FragColor.xyz = N;
}
