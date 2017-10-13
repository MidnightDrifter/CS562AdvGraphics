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


in vec3 normalVec, lightVec, eyeVec, worldPos, reflectVec, transformLightVec,transformEyeVec;
in vec4 shadowCoord, tangent;
in vec2 texCoord;

uniform int objectId;
uniform vec3 diffuse; // Kd
uniform vec3 specular; // Ks
uniform float shininess; // alpha exponent
uniform vec3 Light; // Ii
uniform vec3 Ambient; // Ia

uniform sampler2D shadowTexture; //shadow map thing
uniform sampler2D skydomeTexture; //skybox tex
uniform sampler2D normalMap; //normal map
uniform sampler2D bricksTexture; //brick color tex
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

vec3 BRDF()
{
	return BRDF(normalVec, lightVec, eyeVec, shininess, specular, diffuse);
}

void main()
{
    vec3 N = normalize(normalVec);
    vec3 L = normalize(lightVec);
	vec3 V = normalize(eyeVec);
	vec2 shadowIndex = (shadowCoord.xy) / (shadowCoord.w);

    vec3 Kd = diffuse;   
    
    if (objectId==groundId || objectId==seaId) {
        ivec2 uv = ivec2(floor(200.0*texCoord));
        if ((uv[0]+uv[1])%2==0)
            Kd *= 0.9; }
    


float LN = max(dot(L,N),0.0);

vec3 regularLighting = BRDF(normalVec,lightVec,eyeVec,shininess,specular,Kd);
vec3 outLight = regularLighting*LN*(Light+Ambient);

if(objectId == skyId)
{


vec3 D = V;

vec2 skyTexCoord = vec2(0.5f - atan(D.y,D.x), acos(D.z)/PI);

vec3 skyColor = texture2D(skydomeTexture,skyTexCoord).xyz;

outLight = skyColor; //  Check this  ???

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




if(shadowCoord.w >0 && shadowIndex.x <= 1 && shadowIndex.x >= 0 && shadowIndex.y <= 1 && shadowIndex.y >= 0  &&((shadowCoord.w - texture(shadowTexture,shadowIndex).w) > EPSILON))
{
//Pixel depth = shadowCoord.w
//Light depth = texture(shadowTexture,shadowIndex)
	//outLight  = Ambient;	
	outLight = regularLighting*LN*(Ambient);

}


gl_FragColor.xyz = outLight;






//gl_FragColor.xyz = BRDF(normalVec,lightVec,eyeVec,shininess,specular, Kd) * LN*(Light + Ambient);


   // gl_FragColor.xyz = vec3(0.5,0.5,0.5)*Kd + Kd*max(dot(L,N),0.0);
}
