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
in vec4 shadowCoord;
in vec2 texCoord;
in vec4 tangent;
in vec4 worldPos;

uniform int objectId;
uniform vec3 diffuse; // Kd
uniform vec3 specular; // Ks
uniform float shininess; // alpha exponent
uniform vec3 Light; // Ii
uniform vec3 Ambient; // Ia
//uniform mat4 WorldInverse;
//uniform float tog;
//uniform sampler2D shadowTexture;  //shadowmap
//uniform sampler2D reflectionTextureTop; //top reflection
//uniform sampler2D reflectionTextureBot; //bot reflection
//uniform sampler2D skydomeTexture; //skydome tex
uniform sampler2D normalMap; //normal map
uniform sampler2D parallaxMap;
//uniform sampler2D bricksTexture; //brick color tex


//Get result of depth test somehow, put it into the worldPos gBuffer??



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

vec3 BRDF()
{
	return BRDF(normalVec, lightVec, eyeVec, shininess, specular, diffuse);
}

vec3 BRDFSpec()
{
	return BRDFSpec(normalVec, lightVec, eyeVec, shininess, specular, diffuse);
}

float random(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
/*
    vec3 N = normalize(normalVec);
    vec3 L = normalize(lightVec);
	vec3 V = normalize(eyeVec);
	vec2 shadowIndex = (shadowCoord.xy) / (shadowCoord.w);

    vec3 Kd = diffuse;   
    
    if (objectId==groundId || objectId==seaId) {
        ivec2 uv = ivec2(floor(200.0*texCoord));
        if ((uv[0]+uv[1])%2==0)
            Kd *= 0.9; }
    


	vec2 rand = worldPos.xy;
//	vec3 H = normalize(L+V);
float LN = max(dot(L,N),0.0);
//float HN = max(dot(H,N),0.0);
vec3 reflection = ((2.f*dot(V,N))*N) - V;  //Check this, may need to swap them to the unnormalized vals
vec3 R = normalize(reflection);
vec3 regularLighting = BRDF(normalVec,lightVec,eyeVec,shininess,specular,Kd);
vec3 outLight = regularLighting*LN*(Light+Ambient);
vec3 zero = vec3(0.f, 0.f, 0.f);
vec3 reflectionColor = vec3(1.f,0.5f,0.f);
//float RN = max(dot(R,N),0.f);
float RN = dot(R,N);
//int addReflectionVal =0;


if(objectId == teapotId)
{
	float depth=0.f;
//	addReflectionVal =1;
	vec2 reflectTexCoord = vec2(0.f,0.f);

	if(R.z <0)
		{
			depth = 1.f-R.z;
			reflectTexCoord = vec2((R.x/depth) , (R.y/depth))*0.5f;
			reflectTexCoord += vec2(0.5, 0.5);

			reflectionColor = texture2D(reflectionTextureBot, reflectTexCoord.xy).xyz;

		}

	else
		{
			depth = 1.f+R.z;
			reflectTexCoord = vec2((R.x/depth) , (R.y/depth))*0.5f;
			reflectTexCoord += vec2(0.5, 0.5);
			

			reflectionColor = texture2D(reflectionTextureTop, reflectTexCoord.xy).xyz;
		}

	if(tog > 0)
		{
			outLight += (reflectionColor * RN * BRDFSpec(normalVec,R,eyeVec,shininess,specular,Kd)) * 0.65f;
		}

	else 
		{
	outLight = (reflectionColor*RN*BRDFSpec(normalVec,R,eyeVec,shininess,specular,Kd))  * 0.65f;
		}

	//outLight = reflectionColor;
	


}

else if(objectId == boxId)
{

vec3 textureColor, textureNormal;

textureColor = texture(bricksTexture, texCoord).xyz;


vec3 tan = normalize(tangent.xyz);
vec3 bitan = normalize(cross(tan,N));

vec3 temp = 2.f * texture(normalMap,texCoord).xyz - vec3(1,1,1);


textureNormal = normalize(temp.x * tan + temp.y * bitan + temp.z * N);
float texNL = max(0.f, dot(textureNormal,L));
//outLight = textureColor * texNL * BRDF(textureNormal,L,eyeVec,shininess,specular,Kd);
outLight = (Light+Ambient)*texNL*BRDF(textureNormal,L,eyeVec,shininess,specular,(textureColor));


}

else if(objectId == skyId)
{


vec3 D = V;

vec2 skyTexCoord= vec2(0.5f - atan(D.y,D.x)/(2*PI), acos(D.z)/PI);

vec3 skyColor = texture(skydomeTexture,skyTexCoord).xyz;

outLight = skyColor; //  Check this  ???

}



if(shadowCoord.w >0 && shadowIndex.x <= 1 && shadowIndex.x >= 0 && shadowIndex.y <= 1 && shadowIndex.y >= 0  &&((shadowCoord.w - texture(shadowTexture,shadowIndex).w) > EPSILON))
{
//Pixel depth = shadowCoord.w
//Light depth = texture(shadowTexture,shadowIndex)
	//outLight  = Ambient;	
	//outLight = regularLighting*LN*(Ambient);
	outLight = LN*Ambient*(BRDF(normalVec, lightVec,eyeVec,shininess,zero,zero));


}




gl_FragColor.xyz = outLight;

//gl_FragColor.xyz = texture2D(skydomeTexture,texCoord).xyz;

return;
int t = textureSize(reflectionTextureTop,0).x;
int a = textureSize(reflectionTextureTop,0).y;
int b = textureSize(reflectionTextureBot,0).x;
int s = textureSize(reflectionTextureBot,0).y;


if(textureSize(reflectionTextureTop,0).x <1)
{
	gl_FragColor.xyz = vec3(1.0, 0.0, 0.0);
}

else if(t==(1024) && s == 1024 && a==1024 && b==1024)
{
	gl_FragColor.xyz = vec3(1.0,0.0,1.0);
}

else if(textureSize(reflectionTextureTop,0).x==1)
{
gl_FragColor.xyz = vec3(0.0, 1.0, 0.0);
}



//gl_FragColor.xyz = texture(reflectionTextureTop,worldPos.xy).xyz;





//gl_FragColor.xy = shadowIndex;   //drawing shadowCoord.xy / shadowCoord.w
//gl_FragColor = shadowCoord.wwww /100.f;  //pixel depth / 100
//gl_FragColor = texture(shadowTexture,shadowIndex);// /100.f; //light depth /100

//gl_FragColor.xyz = BRDF(normalVec,lightVec,eyeVec,shininess,specular, Kd) * LN*(Light + Ambient);


   // gl_FragColor.xyz = vec3(0.5,0.5,0.5)*Kd + Kd*max(dot(L,N),0.0);


   */
   vec3 N = normalize(normalVec);
   float worldPosDepth =0.f;  //Need to get the result of the depth test somehow--check shadow thingy probably

    vec3 Kd = diffuse;   
    
    if (objectId==groundId || objectId==seaId) {
        ivec2 uv = ivec2(floor(200.0*texCoord));
        if ((uv[0]+uv[1])%2==0)
            Kd *= 0.9; }

/*
if(objectId == skyId)
{


vec3 D = -1*V;
//vec3 D = V;
//vec2 skyTexCoord= vec2(0.5f - atan(D.y,D.x), -acos(D.z)/PI);  //Flip the acos to flip the skydome


vec2 skyTexCoord= vec2(0.5f - atan(D.y,D.x)/(2*PI), -acos(D.z)/PI);  //Flip this to flip skysphere
vec3 skyColor = texture(skydomeTexture,skyTexCoord).xyz;

//skyColor = vec3(1.f,0.f,0.f);

outLight = skyColor; //  Check this  ???

}
*/





	//PARALLAX MAPPING GOES HERE

	if(objectId == boxId)
	{
	vec3 V = normalize(eyeVec);
	//mat3 TBN;
	vec3 tanNorm = normalize(tangent.xyz);
	vec3 normCopy = N;
	//TBN[0][0]=tanNorm.x;
	//TBN[0][1]=tanNorm.y;
	//TBN[0][2]=tanNorm.z;
	
	
	//TBN[2][0] = N.x;
	//TBN[2][1]=N.y;
	//TBN[2][2]=N.z;
	
	
	vec3 cr = normalize(cross(tanNorm,N));  //tanNorm));
	//TBN[1][0] = cr.x;
	//TBN[1][1]=cr.y;
	//TBN[1][2]=cr.z;



	mat3 TBN = mat3(tanNorm,cr,normCopy);

	int NUM_ATTEMPTS =50;
	float STEP_SIZE = 1.0/NUM_ATTEMPTS;

	mat3 TBNinv = inverse(TBN);
	vec3 transformedViewRay = TBNinv*V;
	vec3 texC = vec3(texCoord,0);
	

	for(int i=1;i<=NUM_ATTEMPTS;i++)  //Arbitrarily choose 30 steps--very small ones
	{
	vec3 depthTest = texC + (i*STEP_SIZE)*transformedViewRay;

	if(depthTest.z - texture2D(parallaxMap,depthTest.xy).x >=0 && depthTest.z - texture2D(parallaxMap,depthTest.xy).x < EPSILON)
	{
	
	N = 2.0*(texture2D(normalMap, depthTest.xy).xyz) - vec3(1,1,1);
	N = N.x * tanNorm  + N.y * cr + N.z *normCopy;  //normalize( TBN*N);

	i=NUM_ATTEMPTS+1;
	
	
	//Look up other textures here as needed
	
	}
	
	
	}


	
	/*

	[... T ...]
	[... B ...]
	[... N ...]    go INTO texture space, so this is TBNinv


	[... ... ...]
	[T    B   N ]
	[... ... ...]  TBN as COLUMNS to go OUT of texture space, so this is TBN

	I think
	Maybe







	Normal map: A normal map is a texture whose RGB
components encode a unit length normal by squeezing the
−1⋯+1 range of unit normals into the 0⋯1 range of
colors. Thus the normal vector (0,0,1) encodes as color
(0.5, 0.5, 1.0) accounting for the characteristic light blue
color of normal maps. When coloring a pixel, modify the
normal N to ̄N and use ̄N for lighting calculations:


(a , b , c) =( lookup normal-map at uv )∗2.0 −( 1,1,1) // Convert to−1⋯+1
̄N = a∗T +b∗B+c∗N // For tangent T , bi-normal B=T×N , and normal N (all unit length)
For this feature to work well, you need a texture-map and normal-map pair that match each other.
The ChippedRedBricks pair shown here and distributed with the framework work well, but many
others can be found on the Internet.



This is for simply reading from the Normal Map!!!!!


T = tangent vector
N = normal vector as usual
B = T x N  // That's a cross product


Matrix TBN is composed of those three vectors as rows.
 [... T ...]
[... B ...]
[... N ...]

It transforms a column vector, say the view vector V into texture
space
  V' = TBN * V
where the X coordinate is the texture u parameter, Y is the texture v
parameter, and Z is the height.





The pixel shader is fired up with a texture coordinate (u,v). You transform the view vector into texture space (via the TBN) to get a 3d vector (a,b,c) in texture space.  Now stepping along the view vector by some amount t is
    (u,v,0) + t*(a,b,c).
The first two coordinates, (u,v) + t*(a,b), are a new texture coordinate, and the third coordinate, t*c, is the height of the view ray at that texture coordinate.  You compare that height with the height from the texture map at that coordinate and decide to continue the search to other t's or not.



	*/
	
	
	
	
	
	
	}


	//PARALLAX MAPPING ENDS HERE









float depth = worldPos.w;
   gl_FragData[0] =  vec4(worldPos.xyz,depth);  //vec4(worldPos.xyz, worldPosDepth);
    gl_FragData[1] = vec4(specular.xyz, shininess);
	 gl_FragData[2] = vec4(Kd.xyz, 0.f);
	  gl_FragData[3] = vec4(N.xyz,objectId);
	  

//gl_FragColor.xyz = worldPos.xyz;
//gl_FragColor.xyz = specular.xyz;
//gl_FragColor.xyz = diffuse.xyz;
//gl_FragColor.xyz = N.xyz;


/*
if(N.x ==1 && N.y == 1 && N.z ==1)
{
gl_FragColor.xyz = vec3(1.f,0.f,0.f);
}

else
	{//  gl_FragColor.xyz = N;
	gl_FragColor.xyz = vec3(0.f,1.f,0.f);
	}
	*/

	  //Toss in some kind of keyboard toggle to shift between outputs?
	  //gl_FragColor = gl_FragData[];


}
