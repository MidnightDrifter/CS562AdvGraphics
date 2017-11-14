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
in vec3 eyeVec;

//uniform int objectId;
//uniform vec3 diffuse;

uniform sampler2D gBuffer0;  //WorldPos.xyz, worldPosDepth
uniform sampler2D gBuffer1;  //specular.xyz, shininess
uniform sampler2D gBuffer2;  //diffuse.xyz
uniform sampler2D gBuffer3;  //normalVec.xyz


uniform sampler2D skydomeTexture;
uniform sampler2D irradianceMap;

uniform int HamN;
//uniform sampler2D 
/*
uniform HammersleyBlock {
int HamN;
float hammersley[2*100]; };
*/



uniform vec3 ambient;

uniform int width;
uniform int height;
uniform int skyWidth, skyHeight;

uniform mat4 WorldInverse;

uniform float exposure;
uniform float contrast;


//Shamelessly pulled from online
vec2 Hammersley(uint i, uint N)
{
  return vec2(
    float(i) / float(N),
    float(bitfieldReverse(i)) * 2.3283064365386963e-10
  );
}



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


float LH = max(0,dot(L,H));
float denominator = 4* pow(LH,2);

return 1/denominator;

//return 1/(4*pow(max(0,dot(L,H)),2));

}


float dValue(vec3 lVec, vec3 eVec, vec3 nVec, float alpha)
{

vec3 N = normalize(nVec);
vec3 L = normalize(lVec);
vec3 V = normalize(eVec);
vec3 H = normalize(lVec+eVec);

float NH = max(0,dot(N,H));
float numerator = 2+alpha;
float numerator2 = pow(NH,alpha);
float denominator = 2*PI;

return (numerator * numerator2)/denominator;    //((2+alpha)/(PI*2))*(pow(max(dot(N,H),0),alpha));


}


vec3 fValue(vec3 lVec, vec3 eVec, vec3 nVec, vec3 spec)
{

vec3 N = normalize(nVec);
vec3 L = normalize(lVec);
vec3 V = normalize(eVec);
vec3 H = normalize(lVec+eVec);

float LH = max(0,dot(L,H));
float powerTerm = pow(1-LH,5);
return spec + ( (1-spec) * (powerTerm) );


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
	//gl_FragColor.xy = myPixelCoordinate;
	//return;
//	vec3 worldPos = texture2D(gBuffer0,myPixelCoordinate).xyz;
//	float  worldPosDepth = texture2D(gBuffer0,myPixelCoordinate).w;
	
	vec3 specular = texture2D(gBuffer1, myPixelCoordinate).xyz;
	float shininess = texture2D(gBuffer1, myPixelCoordinate).w;

  	vec3 diffuse = texture2D(gBuffer2, myPixelCoordinate).xyz;
	//vec3 L = normalize(lightVec);
	vec3 N = normalize(texture2D(gBuffer3,myPixelCoordinate).xyz);
	//float LN = max(dot(L,N),0.0f);




	//Lin. Color Space for input colors
	diffuse = pow(diffuse,vec3(2.2));
	specular = pow(specular,vec3(2.2));


//	gl_FragColor.xyz = N;
//	gl_FragColor.xyz = diffuse;
//	gl_FragColor.xyz = specular;
//  gl_FragColor.xyz = worldPos.xyz;

//Lin. Color Space & Tone Mapping

//	vec2 myPixelCoordinate = vec2(gl_FragCoord.x/ width, gl_FragCoord.y/height);  

	vec3 worldPos = texture2D(gBuffer0,myPixelCoordinate).xyz;
		
		vec3 V = normalize((WorldInverse * vec4(0.f, 0.f, 0.f, 1.f)).xyz-worldPos);
vec3 outColor = vec3(0,0,0);
//vec3 inColor = texture2D(skydomeTexture, skyTexCoord).xyz;
//inColor = pow(inColor, vec3(2.2));

		vec3 R = 2*dot(N,V)*N-V;
		R = normalize(R);	


if(texture2D(gBuffer3,myPixelCoordinate).w == skyId)
{




		
vec3 D = -1*V;
//vec3 D = V;
//vec2 skyTexCoord= vec2(0.5f - atan(D.y,D.x), -acos(D.z)/PI);  //Flip the acos to flip the skydome


vec2 skyTexCoord= vec2(0.5f - atan(D.y,D.x)/(2*PI), acos(D.z)/PI);  //Flip this to flip skysphere
vec3 skyColor = texture(skydomeTexture,skyTexCoord).xyz;

//skyColor = vec3(1.f,0.f,0.f);

//outColor = pow(skyColor, vec3(contrast/2.2));
outColor = pow(skyColor, vec3(2.2));
/*
		if(outColor.x > 1 || outColor.x <0)
		{
		outColor.x = pow( (exposure*outColor.x)/((exposure*outColor.x) + 1) , contrast/2.2);
		}

		
		if(outColor.y > 1 || outColor.y <0)
		{
		outColor.y = pow((exposure*outColor.y)/((exposure*outColor.y) + 1) , contrast/2.2);
		}

		
		if(outColor.z > 1 || outColor.z <0)
		{
		outColor.z = pow((exposure*outColor.z)/((exposure*outColor.z) + 1), contrast/2.2);
		}
*/		

		outColor = (vec3(exposure)*outColor) / (vec3(exposure)*outColor + vec3(1));
		outColor = pow(outColor,vec3(contrast/2.2));

		
//glFrag_Color.xyz = pow(skyColor, vec3(contrast/2.2)); //  Check this  ???

gl_FragColor.xyz = outColor.xyz;

//Lin. Color Space & Tone Mapping




return;








}


		vec3 A = normalize(cross(vec3(0,0,1), R));
		vec3 B = normalize(cross(R,A));
		
		vec3 monteCarloSum = vec3(0,0,0);
		//float level;

		//float monteCarloSum=0;

		//vec3 fValue = spec + ((1-spec)*(pow((1-LH),5)));

		//float gValue = 1 / (pow(LH,2)*4);   //Raised to power of 2, no need to care about negative vals -- maybe div. by 0 though


		for(int i =0; i<HamN;i++)
		{
		vec2 randTexCoord = Hammersley(i,HamN);   // vec2(hammersley[i], hammersley[i+1]);
	  	randTexCoord.y = (  acos(  pow(randTexCoord.y, (1/(shininess+1)  )  )  )  )/PI;
		
		vec3 L = normalize( vec3(    cos(   2*PI*(0.5-randTexCoord.x)  )   *  sin(  PI*randTexCoord.y  )    ,    sin(2*PI*(0.5-randTexCoord.x))*   sin(PI*randTexCoord.y)    ,      cos(PI*randTexCoord.y)	)   );
		vec3 wK = normalize(L.x * A + L.y * B + L.z * R);

		vec2 wKTexCoord = vec2(0.5-(atan(wK.y,wK.x)/(2*PI)), acos(wK.z)/PI);

		//vec2 LTexCoord =  vec2(0.5-(atan(L.y,L.x)/(2*PI)), acos(L.z)/PI);


	//	float bigLog = log2(1.0*width*height/HamN);
	//	float dLog = log2(1.0*width*height/HamN);

	//Using SCREEN SPACE width & height
	//	float	level = (  (0.5)* log2(1.0*width*height/HamN)  ) - ( 0.5* log2(dValue(wK,V,N,shininess)/4)   ) ;  //Check this--might use L instead of wK

	
			//Using HDR dimensions
		float level = (  (0.5)* log2(1.0*skyWidth*skyHeight/HamN)  ) - ( 0.5* log2(dValue(wK,V,N,shininess)/4)   ) ;  //Check this--might use L instead of wK

	
		vec3 lightIntensity = textureLod(skydomeTexture, wKTexCoord,level).xyz;   //Check this line later--might need to raise to that 2.2 power
		//lightIntensity = texture(skydomeTexture,wKTexCoord).xyz;
		lightIntensity = pow(lightIntensity,vec3(2.2));


		//vec3 lightIntensity = texture(skydomeTexture,wKTexCoord).xyz;
		//vec3 H = normalize(wK+V);

		monteCarloSum += (gValue(wK,V,N) * fValue(wK,V,N,specular)*lightIntensity*max(0,dot(wK,N)));
		//  monteCarloSum += (gValue(wK,V,N) * fValue(wK,V,N,specular)*lightIntensity*(dot(wK,N)));	
		}



		//monteCarloSum /= vec3(HamN);   //Specular bit!!
		monteCarloSum.x /= (2*HamN);
		monteCarloSum.y /= (2*HamN);
		monteCarloSum.z /= (2*HamN);




		//Diffuse bit--just read from irradiance map
		vec2 irradianceTexCoord = vec2(0.5-(atan(N.y,N.x)/(2*PI)), acos(N.z)/PI);   //vec2( 0.5−(atan(N.y ,N.x)/(2*PI) ), acos(N.z)/PI );
		vec3 diffuseApprox = (diffuse/(PI)) *  texture(irradianceMap, irradianceTexCoord).xyz;    //pow(texture(irradianceMap, irradianceTexCoord).xyz, vec3(2.2));
		//diffuseApprox = pow(diffuseApprox, vec3(2.2));
		//diffuseApprox = (diffuse/PI) * (pow(texture(irradianceMap, irradianceTexCoord).xyz,vec3(2.2)));



		outColor = diffuseApprox + monteCarloSum;
		//outColor = diffuseApprox;
	//	outColor = monteCarloSum;


	//	outColor = pow(texture(irradianceMap,irradianceTexCoord).xyz, vec3(2.2));
		outColor = (  (exposure*outColor)  /  (  (exposure*outColor) + vec3(1) )   );
		outColor = pow(outColor,vec3(contrast/2.2));
		
		/*
		if(outColor.x > 1 || outColor.x <0)
		{
		outColor.x = pow( (exposure*outColor.x)/((exposure*outColor.x) + 1) , contrast/2.2);
		}

		
		if(outColor.y > 1 || outColor.y <0)
		{
		outColor.y = pow((exposure*outColor.y)/((exposure*outColor.y) + 1) , contrast/2.2);
		}

		
		if(outColor.z > 1 || outColor.z <0)
		{
		outColor.z = pow((exposure*outColor.z)/((exposure*outColor.z) + 1), contrast/2.2);
		}
		*/
		//outColor = pow(outColor, vec3(contrast/2.2));


		gl_FragColor.xyz = outColor;
	//gl_FragColor.xy = irradianceTexCoord;

	//vec3 temp = (vec3(diffuse)/PI)*texture(irradianceMap, irradianceTexCoord).xyz;

	//temp = (vec3(exposure) * temp) / (vec3(exposure) * temp + vec3(1));
	//gl_FragColor.xyz = pow(temp, vec3(contrast/2.2));

		//gl_FragColor.xyz = (vec3(1)/PI)*5*texture(irradianceMap, irradianceTexCoord).xyz;
  //gl_FragColor.xyz = ambient;

  //gl_FragColor.xyz = *texture(irradianceMap,myPixelCoordinate).xyz; 

 // gl_FragColor.xyz = N;
}
