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

const float e = 2.7182818284590452353602874713527;



//in vec3 normalVec, lightVec;
//in vec2 texCoord;

//uniform int objectId;
//uniform vec3 diffuse;

in vec4 pos;
uniform float maxDepth, minDepth;
void main()
{
  //  vec3 N = normalize(normalVec);
  // vec3 L = normalize(lightVec);

  //  vec3 Kd = diffuse;   
    /*
    if (objectId==groundId || objectId==seaId) {
        ivec2 uv = ivec2(floor(200.0*texCoord));
        if ((uv[0]+uv[1])%2==0)
            Kd *= 0.9; }
    */

	
	float depth = (pos.w - minDepth)/(maxDepth-minDepth);

	gl_FragData[0]=vec4(pos.xyz,exp(e,c*depth));
	
	
	
	
	
	//gl_FragColor = pos;
    //gl_FragColor.xyz = vec3(0.5,0.5,0.5)*Kd + Kd*max(dot(L,N),0.0);

//	gl_FragColor.xyz = vec3(1.0,0.0,0.0);
}
