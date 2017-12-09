//////////////////////////////////////////////////////////////////////
// Defines and draws a scene.  There are two main procedures here:
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////

#include "math.h"
#include <fstream>
#include <stdlib.h>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;

#include <freeglut.h>
#include <glu.h>                // For gluErrorString

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
using namespace glm;

#include "shader.h"
#include "shapes.h"
#include "scene.h"
#include "object.h"
#include "texture.h"
#include "transform.h"

const float PI = 3.14159f;
const float rad = PI/180.0f;    // Convert degrees to radians

MAT4 Identity;
MAT4 Identity1;
MAT4 Identity2;


const float grndSize = 100.0;    // Island radius;  Minimum about 20;  Maximum 1000 or so
const int   grndTiles = 100;//int(grndSize);
const float grndOctaves = 4.0;  // Number of levels of detail to compute
const float grndFreq = 0.03;    // Number of hills per (approx) 50m
const float grndPersistence = 0.03; // Terrain roughness: Slight:0.01  rough:0.05
const float grndLow = -3.0;         // Lowest extent below sea level
const float grndHigh = 5.0;        // Highest extent above sea level

// Simple procedure to print a 4x4 matrix -- useful for debugging
void PrintMat(const MAT4& m)
{
    for (int i=0;  i<4;  i++)
        for (int j=0;  j<4;  j++)
            printf("%9.4f%c", m[i][j], j==3?'\n':' ');
    printf("\n");
}

////////////////////////////////////////////////////////////////////////
// This macro makes it easy to sprinkle checks for OpenGL errors
// through out your code.  Most OpenGL calls can record errors, and a
// careful programmer will check the error status *often*, perhaps as
// often as right after every OpenGL call.  At the very least, once
// per refresh will tell you if something is going wrong.
#define CHECKERROR\
 {GLenum err = glGetError();\
  if (err != GL_NO_ERROR) {\
fprintf(stdout, "OpenGL error (at line %d): %s\n", __LINE__, gluErrorString(err));}}

//exit(-1);}\
}

vec3 HSV2RGB(const float h, const float s, const float v)
{
    if (s == 0.0)
        return vec3(v,v,v);

    int i = (int)(h*6.0);
    float f = (h*6.0f) - i;
    float p = v*(1.0f - s);
    float q = v*(1.0f - s*f);
    float t = v*(1.0f - s*(1.0f-f));
    if (i%6 == 0)     return vec3(v,t,p);
    else if (i == 1)  return vec3(q,v,p);
    else if (i == 2)  return vec3(p,v,t);
    else if (i == 3)  return vec3(p,q,v);
    else if (i == 4)  return vec3(t,p,v);
    else if (i == 5)  return vec3(v,p,q);
}

Object* SphereOfSpheres(Shape* SpherePolygons)
{
    Object* ob = new Object(NULL, nullId);
    
    for (float angle=0.0;  angle<360.0;  angle+= 18.0)
        for (float row=0.075;  row<PI/2.0;  row += PI/2.0/6.0) {   
            vec3 color = HSV2RGB(angle/360.0, 1.0f-2.0f*row/PI, 1.0f);

            Object* sp = new Object(SpherePolygons, spheresId,
                                    color, vec3(1.0, 1.0, 1.0), 1.0);
            float s = sin(row);
            float c = cos(row);
            ob->add(sp, Rotate(2,angle)*Translate(c,0,s)*Scale(0.075*c,0.075*c,0.075*c));
        }
    return ob;
}

////////////////////////////////////////////////////////////////////////
// Called regularly to update the atime global variable.
float atime = 0.0;
void animate(int value)
{
    atime = 360.0*glutGet((GLenum)GLUT_ELAPSED_TIME)/36000;
    glutPostRedisplay();

    // Schedule next call to this function
    glutTimerFunc(30, animate, 1);
}




void Scene::SphericalHarmonics(int irradianceWidth, int irradianceHeight, int hdrWidth, int hdrHeight, const std::vector<basicVec3>& vals)
{

	const float PI = 3.1415926535897932384626433832795;
	float A[3] = { PI, 2 * PI / 3, PI / 4 };
	
	vec3 sphericalHarmonicsCoefficients[9]={ vec3(0),vec3(0),vec3(0),vec3(0),vec3(0),vec3(0),vec3(0),vec3(0),vec3(0) };


	//float angles[2] = { 0,0 };
	vec3 light(0, 0, 0);
	float deltas[2] = { PI / hdrHeight, 2*PI / hdrWidth };
	float angles[2] = { 0,0 };
	vec3 anglesToVector(0, 0, 0);

	for (int i = 0; i < hdrHeight; i++)
	{
		for (int j = 0; j < hdrWidth; j++)
		{
			light.x = vals[hdrWidth * i + j].x;
			light.y = vals[hdrWidth * i + j].y;
			light.z = vals[hdrWidth * i + j].z;

			angles[0] = (PI* (i + 0.5)) / hdrHeight;
			angles[1] = (2 * PI*(j + 0.5)) / hdrWidth;
			
			anglesToVector.x = sinf(angles[0]) * cosf(angles[1]);
			anglesToVector.y = sinf(angles[0]) * sinf(angles[1]);
			anglesToVector.z = cosf(angles[1]);



			sphericalHarmonicsCoefficients[0] = light *  A[0] * sinf(angles[0]) * (0.5f * sqrtf(PI/2)) * deltas[0] * deltas[1];
			sphericalHarmonicsCoefficients[1] = light *  A[1] * sinf(angles[0]) * (anglesToVector.y * (0.5f * sqrtf(3/PI)))* deltas[0] * deltas[1];
			sphericalHarmonicsCoefficients[2] = light *  A[1] * sinf(angles[0]) * (anglesToVector.z * (0.5f * sqrtf(3/PI)))* deltas[0] * deltas[1];
			sphericalHarmonicsCoefficients[3] = light *  A[1] * sinf(angles[0]) * (anglesToVector.x * (0.5f * sqrtf(3/PI)))* deltas[0] * deltas[1];
			sphericalHarmonicsCoefficients[4] = light *  A[2] * sinf(angles[0]) * (anglesToVector.x * anglesToVector.y * 0.5f*sqrtf(15/PI))* deltas[0] * deltas[1];
			sphericalHarmonicsCoefficients[5] = light *  A[2] * sinf(angles[0]) * (anglesToVector.y * anglesToVector.z * 0.5f * sqrtf(15 / PI))* deltas[0] * deltas[1];
			sphericalHarmonicsCoefficients[6] = light *  A[2] * sinf(angles[0]) * ((3 * anglesToVector.z * anglesToVector.z - 1) * (0.25f * sqrtf(5/PI)))* deltas[0] * deltas[1];
			sphericalHarmonicsCoefficients[7] = light *  A[2] * sinf(angles[0]) * (anglesToVector.x * anglesToVector.z * (0.5f * sqrtf(15/PI)))* deltas[0] * deltas[1];
			sphericalHarmonicsCoefficients[8] = light *  A[2] * sin(angles[0]) * ((anglesToVector.x * anglesToVector.x - anglesToVector.y * anglesToVector.y) * (0.25f * sqrtf(15/PI)))* deltas[0] * deltas[1];
		}
	}





}



////////////////////////////////////////////////////////////////////////
// InitializeScene is called once during setup to create all the
// textures, shape VAOs, and shader programs as well as a number of
// other parameters.
void Scene::InitializeScene()
{
	
	prevTime = glutGet((GLenum)GLUT_ELAPSED_TIME);
	curTime = prevTime;
	float sum = 0;

	for (int i = -kernelWidth / 2; i <= kernelWidth / 2; i++)
	{
		kernelWeights.push_back( exp( ((-i*i) / (2.f*s*s))));
		//sum += kernelWeights[(i + kernelWidth / 2)];
		
	}

	for (int i = 0; i < kernelWeights.size(); i++)
	{
		sum += kernelWeights[i];
	}

	for (int i = 0; i < kernelWeights.size(); i++)
	{
		kernelWeights[i] = kernelWeights[i]/ sum;
	}

	parallaxMap = new Texture("textures//DisplacementMap.png");
	normalMap = new Texture("textures//NormalMap.png");

	//Proj. 3 Hammersley points -- generate them once at the start, or make them each and every pass?
	
	//block.N = N; // N=20 ... 40 or whatever …
	//block.hammersley = std::vector<float>(block.N*2);
//	int kk;
	
	

	HamBlock.HamN = HammersleyN2;

	HamBlock.hammersley = new float[2 * HamBlock.HamN];

	/*
	std::vector<float> tempFloats(2 * HamBlock.HamN);


	

	for (int k = 0; k < HamBlock.HamN; k++) {
		int kk = k;
		float u = 0.f;
		//for (float p = 0.5f, int kk = k, float u = 0.0f; kk; p *= 0.5f, kk >>= 1)
		for (float p = 0.5f; kk; p *= 0.5f)
		{
			if (kk & 1)
			{
				u += p;
			}
			kk >>= 1;
		}
			float v = (k + 0.5f) / HamBlock.HamN;
			tempFloats[pos++] = u;
			tempFloats[pos++] = v;
		
		

	}

	for(int x =0;x<2*HamBlock.HamN;x++)
	{
		HamBlock.hammersley[x] = tempFloats[x];
	}
	*/
	
	//Check this--scope of 'u' might be off, it's a bit unclear in the handout




	//Sanity check
//	sum = 0;
	//for (int i = 0; i < 2 * kernelWidth + 1; i++)
//	{
//		sum += kernelWeights[i];
//	}

	std::string hdrTex = "textures//14-Hamarikyu_Bridge_B_3k.hdr";
	std::string hdrTexIrr = "textures//14-Hamarikyu_Bridge_B_3k.irr.hdr";
	

 		skydome = new Texture("textures//sky.jpg");
		HDRskydome = new Texture();
		HDRskydome->MakeHDRTexture("textures//14-Hamarikyu_Bridge_B_3k.hdr");
		CHECKERROR;
		irradianceMap = new Texture();
		irradianceMap->MakeHDRTexture("textures//14-Hamarikyu_Bridge_B_3k.irr.hdr");

    //glEnable(GL_DEPTH_TEST);
    CHECKERROR;

    // FIXME: This is a good place for initializing the transformation
    // values.

//	glBlendFunc(GL_ONE, GL_ONE);
//	glBlendEquation(GL_FUNC_ADD);
	//glDisable(GL_BLEND);
    objectRoot = new Object(NULL, nullId);
	objectRootNoTeapot = new Object(NULL, nullId);
	FSQ = new Object(NULL, nullId);

	FSQ->add(new Object(new Quad(1),-1,ambientLight,ambientLight,1.f), Scale(1000.f,1000.f,1000.f));




    // Set the initial f position parammeters
    lightSpin = 98.0;
    lightTilt = -80.0;
   // lightDist = 1000000.0;
	lightDist = 225.0f;
    // Enable OpenGL depth-testing
    glEnable(GL_DEPTH_TEST);

    // FIXME:  Change false to true to randomize the room's position.
    ground =  new ProceduralGround(grndSize, grndTiles, grndOctaves, grndFreq,grndPersistence, grndLow, grndHigh, false);

    basePoint = ground->highPoint;

    // Create the lighting shader program from source code files.
  /*
	lightingProgram = new ShaderProgram();
	lightingProgram->AddShader("lighting-BRDF&Texture Proj2.vert", GL_VERTEX_SHADER);
	lightingProgram->AddShader("lighting-BRDF&Texture Proj2.frag", GL_FRAGMENT_SHADER);
	//lightingProgram->AddShader("lighting.vert", GL_VERTEX_SHADER);
	//lightingProgram->AddShader("lighting.frag", GL_FRAGMENT_SHADER);
	



    glBindAttribLocation(lightingProgram->programId, 0, "vertex");
    glBindAttribLocation(lightingProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(lightingProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(lightingProgram->programId, 3, "vertexTangent");
    lightingProgram->LinkProgram();
	*/
	
	shadowProgram = new ShaderProgram();
	shadowProgram->AddShader("shadowProj3.vert", GL_VERTEX_SHADER);
	shadowProgram->AddShader("shadowProj3.frag", GL_FRAGMENT_SHADER);

	glBindAttribLocation(shadowProgram->programId, 0, "vertex");
	//glBindAttribLocation(shadowProgram->programId, 1, "vertexNormal");
	//glBindAttribLocation(shadowProgram->programId, 2, "vertexTexture");
	//glBindAttribLocation(shadowProgram->programId, 3, "vertexTangent");
	shadowProgram->LinkProgram();
	/*
	reflectionProgramTop = new ShaderProgram();

	reflectionProgramTop->AddShader("reflectionShaderTop.vert", GL_VERTEX_SHADER);
	reflectionProgramTop->AddShader("reflectionShaderTop.frag", GL_FRAGMENT_SHADER);

	//CHECKERROR;
	glBindAttribLocation(reflectionProgramTop->programId, 0, "vertex");
	glBindAttribLocation(reflectionProgramTop->programId, 1, "vertexNormal");
	glBindAttribLocation(reflectionProgramTop->programId, 2, "vertexTexture");
	glBindAttribLocation(reflectionProgramTop->programId, 3, "vertexTangent");

	reflectionProgramTop->LinkProgram();


	reflectionProgramBot = new ShaderProgram();

	reflectionProgramBot->AddShader("reflectionShaderBot.vert", GL_VERTEX_SHADER);
	reflectionProgramBot->AddShader("reflectionShaderBot.frag", GL_FRAGMENT_SHADER);


	glBindAttribLocation(reflectionProgramBot->programId, 0, "vertex");
	glBindAttribLocation(reflectionProgramBot->programId, 1, "vertexNormal");
	glBindAttribLocation(reflectionProgramBot->programId, 2, "vertexTexture");
	glBindAttribLocation(reflectionProgramBot->programId, 3, "vertexTangent");

	reflectionProgramBot->LinkProgram();

	*/
	
	shadowBlurComputeShader = new ShaderProgram();
	shadowBlurComputeShader->AddShader("shadowBlurCompute.comp", GL_COMPUTE_SHADER);


	shadowBlurComputeShader->LinkProgram();
	
	
	ambientOcclusionBilateralBlurShader = new ShaderProgram();
	ambientOcclusionBilateralBlurShader->AddShader("ambientOcclusionBilateralBlurCompute.comp", GL_COMPUTE_SHADER);
	
	ambientOcclusionBilateralBlurShader->LinkProgram();


	ambientOcclusionShader = new ShaderProgram();

	ambientOcclusionShader->AddShader("ambientOcclusion.vert", GL_VERTEX_SHADER);
	ambientOcclusionShader->AddShader("ambientOcclusion.frag", GL_FRAGMENT_SHADER);


	glBindAttribLocation(ambientOcclusionShader->programId, 0, "vertex");
	
	
	ambientOcclusionShader->LinkProgram();



	
	gBufferShader = new ShaderProgram();

	gBufferShader->AddShader("gBuffer.vert", GL_VERTEX_SHADER);
	gBufferShader->AddShader("gBuffer.frag", GL_FRAGMENT_SHADER);


	glBindAttribLocation(gBufferShader->programId, 0, "vertex");
	glBindAttribLocation(gBufferShader->programId, 1, "vertexNormal");
	glBindAttribLocation(gBufferShader->programId, 2, "vertexTexture");
	glBindAttribLocation(gBufferShader->programId, 3, "vertexTangent");

	gBufferShader->LinkProgram();



	gBufferLocalLighting = new ShaderProgram();

	gBufferLocalLighting->AddShader("gBufferLocalLighting.vert", GL_VERTEX_SHADER);
	gBufferLocalLighting->AddShader("gBufferLocalLighting.frag", GL_FRAGMENT_SHADER);


	glBindAttribLocation(gBufferLocalLighting->programId, 0, "vertex");
	glBindAttribLocation(gBufferLocalLighting->programId, 1, "vertexNormal");
	glBindAttribLocation(gBufferLocalLighting->programId, 2, "vertexTexture");
	glBindAttribLocation(gBufferLocalLighting->programId, 3, "vertexTangent");

	gBufferLocalLighting->LinkProgram();


	gBufferGlobalLighting = new ShaderProgram();

	gBufferGlobalLighting->AddShader("gBufferGlobalLighting.vert", GL_VERTEX_SHADER);
	gBufferGlobalLighting->AddShader("gBufferGlobalLighting.frag", GL_FRAGMENT_SHADER);


	glBindAttribLocation(gBufferGlobalLighting->programId, 0, "vertex");
//	glBindAttribLocation(gBufferGlobalLighting->programId, 1, "vertexNormal");
//	glBindAttribLocation(gBufferGlobalLighting->programId, 2, "vertexTexture");
//	glBindAttribLocation(gBufferGlobalLighting->programId, 3, "vertexTangent");

	gBufferGlobalLighting->LinkProgram();




	gBufferAmbientLighting = new ShaderProgram();

	gBufferAmbientLighting->AddShader("gBufferAmbientLighting.vert", GL_VERTEX_SHADER);
	gBufferAmbientLighting->AddShader("gBufferAmbientLighting.frag", GL_FRAGMENT_SHADER);


	glBindAttribLocation(gBufferAmbientLighting->programId, 0, "vertex");
//	glBindAttribLocation(gBufferAmbientLighting->programId, 1, "vertexNormal");
//	glBindAttribLocation(gBufferAmbientLighting->programId, 2, "vertexTexture");
//	glBindAttribLocation(gBufferAmbientLighting->programId, 3, "vertexTangent");

	gBufferAmbientLighting->LinkProgram();
	
	/*

	basicOutputShader = new ShaderProgram();

	basicOutputShader->AddShader("basicOutputShader.vert", GL_VERTEX_SHADER);
	basicOutputShader->AddShader("basicOutputShader.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(basicOutputShader->programId, 0, "vertex");



	basicOutputShader->LinkProgram();

	*/




	CHECKERROR;
	shadowTexture = new FBO();
	shadowTexture->CreateFBO(1024, 1024);

	reflectionTextureTop = new FBO();
	reflectionTextureTop->CreateFBO(1024, 1024);


	reflectionTextureBot = new FBO();
	reflectionTextureBot->CreateFBO(1024, 1024);


	blurredShadowTexture = new FBO();
	blurredShadowTexture->CreateFBO(1024, 1024);

	shadowBlurPureTexture = new Texture(1024, 1024);


	gBuffer = new FBO();
	gBuffer->CreateGBuffer(width, height);

	screenOutput = new FBO();
	screenOutput->CreateFBO(width, height);
	
	ambientOcclusionTexture = new FBO();
	ambientOcclusionTexture->CreateFBO(width, height);
	
	ambientOcclusionBlurredTexture = new FBO();
	ambientOcclusionBlurredTexture->CreateFBO(width, height);

//	test = new Texture("grass.jpg");

//	bricksNormalTexture = new Texture("textures//Standard_red_pxr128_normal.png");
//	bricksTexture = new Texture("textures//Standard_red_pxr128.png");
    // Create all the Polygon shapes
   // Shape* TeapotPolygons =  new Teapot(12);  //Replace teapot with sphere
	Shape* TeapotPolygons = new Teapot(12);
    Shape* BoxPolygons = new Ply("box.ply");
    Shape* SpherePolygons = new Sphere(32);
    Shape* GroundPolygons = ground;
    Shape* SeaPolygons = new Plane(2000.0, 50);
	
	


    // Create all the models from which the scene is composed.  Each
    // is created with a polygon shape (possible NULL), a
    // transformation, and the curface lighting parameters Kd, Ks, and
    // alpha.
    Object* anim = new Object(NULL, nullId);
    //Replace teapot with sphere for Earth
	//Object* teapot = new Object(TeapotPolygons, teapotId, vec3(0.5, 0.5, 0.1), vec3(0.5, 0.5, 0.5), 120);
//	Object* teapot = new Object(TeapotPolygons, teapotId, vec3(0.5, 0.5, 0.1), vec3(0.5, 0.5, 0.5), 120);
	Object* teapot = new Object(TeapotPolygons, teapotId, teapotDiffuse, teapotSpecular, teapotShininess);
	Object* podium = new Object(BoxPolygons, boxId,
                                vec3(0.25, 0.25, 0.1), vec3(0.3, 0.3, 0.3), 10);
    
    Object* spheres = SphereOfSpheres(SpherePolygons);
    
    
    Object* sky = new Object(SpherePolygons, skyId,
                             vec3(), vec3(), 0);

    Object* ground = new Object(GroundPolygons, groundId,
                                vec3(0.3, 0.2, 0.1), vec3(0.0, 0.0, 0.0), 1);

    Object* sea = new Object(SeaPolygons, seaId,
                             vec3(0.3, 0.3, 1.0), vec3(1.0, 1.0, 1.0), 120);
 localLights = new Object(NULL, nullId);

 /*

	for (int i = 0; i <= numLocalLights; i++)
	{
		//Start at ~ -10 x, increment up little by little in positive x towards +10
		//Y is the up direction, right?  Or was it Z?
	//	Shape* s = new Sphere(localLightRadius);
		Object* lightSphere = new Object(SpherePolygons, localLightsId, vec3(0, 0, 0), vec3(0, 0, 0), 120.f);
		localLights->add( lightSphere, Translate(-50 + (100.f * i/numLocalLights), 3.f, 3.f)*Scale(localLightRadius, localLightRadius,localLightRadius));

	}

	*/
 MAT4 localLightRadiusMat = Scale(localLightRadius, localLightRadius, localLightRadius);

 Object* lightSphere = new Object(SpherePolygons, localLightsId, vec3(0, 0, 0), vec3(0, 0, 0), 1.f);
 localLights->add(lightSphere, localLightRadiusMat);

 for (int i = 0; i < numLocalLights; i++)
 {
	 Object* bob = new Object(SpherePolygons, localLightsId, vec3(0, 0, 0), vec3(0, 0, 0), 1.f);
	 localLights->add(bob, Translate(i-(numLocalLights/2), 0, 0) * localLightRadiusMat);
 }


 //Just one local light for sanity check


    // FIXME: This is where you could read in all the textures and
    // associate them with the various objects just created above
    // here.

    // Scene is composed of sky, ground, sea, and ... models
    objectRoot->add(sky, Scale(500.0, 500.0, 500.0));
    
	
//	objectRoot->add(ground);
//    objectRoot->add(sea);


  objectRootNoTeapot->add(ground);
  objectRootNoTeapot->add(sea);




    // Two models have rudimentary animation (constant rotation on Z)
    animated.push_back(anim);

   objectRoot->add(podium, Translate(0,0,0));
    objectRoot->add(anim, Translate(0,0,0));
    objectRoot->add(teapot, Translate(0.0,0,1.5)*TeapotPolygons->modelTr);
	//objectRoot->add(earth, Translate(0.0, 0, 1.5)*SpherePolygons->modelTr);
    anim->add(spheres, Translate(0.0,0,0.0)*Scale(20,20,20));
    

	//Add every object EXCEPT the teapot to another object, draw that for reflection passes
	// Scene is composed of sky, ground, sea, and ... models
	objectRootNoTeapot->add(sky, Scale(500.0, 500.0, 500.0));


	// Two models have rudimentary animation (constant rotation on Z)
	//animated.push_back(anim);

	objectRootNoTeapot->add(podium, Translate(0, 0, 0));
	objectRootNoTeapot->add(anim, Translate(0, 0, 0));


    // Schedule first timed animation call
    glutTimerFunc(30, animate, 1);



	//Blend settings??
	glBlendFunc(GL_ONE, GL_ONE);
	CHECKERROR
	//glBlendEquation(GL_FUNC_ADD);




	


	//NEED TO NORMALIZE THESE TO SUM TO 1 LATER!!


    CHECKERROR;
}

////////////////////////////////////////////////////////////////////////
// Procedure DrawScene is called whenever the scene needs to be drawn.
void Scene::DrawScene()
{
	int pos = 0;
	std::vector<float> tempFloats(2 * HamBlock.HamN);
	for (int k = 0; k < HamBlock.HamN; k++) {
		int kk = k;
		float u = 0.f;
		//for (float p = 0.5f, int kk = k, float u = 0.0f; kk; p *= 0.5f, kk >>= 1)
		for (float p = 0.5f; kk; p *= 0.5f)
		{
			if (kk & 1)
			{
				u += p;
			}
			kk >>= 1;
		}
		float v = (k + 0.5f) / HamBlock.HamN;
		tempFloats[pos++] = u;
		tempFloats[pos++] = v;



	}

	for (int x = 0; x<2 * HamBlock.HamN; x++)
	{
		HamBlock.hammersley[x] = tempFloats[x];
	}



    // Calculate the light's position.
    float lPos[4] = {
       basePoint.x+lightDist*cosf(lightSpin*rad)*sinf(lightTilt*rad),
       basePoint.y+lightDist*sinf(lightSpin*rad)*sinf(lightTilt*rad),
       basePoint.z+lightDist*cosf(lightTilt*rad),
       1.0 };

    // Set the viewport, and clear the screen
	/*
	printf("basePoint (x,y,z):  (%f,   %f   %f) \n",basePoint.x, basePoint.y, basePoint.z);
	printf("lightDist:  %f \n",lightDist);
	printf("lightSpin:  %f \n", lightSpin);
	printf("lightTilt:  %f \n", lightTilt);
	printf("rad:  %f  \n", rad);
	*/



    // Compute Viewing and Perspective transformations.
    MAT4 WorldProj, WorldView, WorldInverse, LightProj, LightView,LightInverse, ShadowMatrix;

    // FIXME: When you are ready to try interactive viewing, replace
    // the following hardcoded values for WorldProj and WorldView with
    // transformation matrices calculated from variables such as spin,
    // tilt, tr, basePoint, ry, front, and back.
	if (isToggled)
	{
		float framerate = (curTime - prevTime) /1000.f;
		float tspin = PI * spin / 180.f;  //Needed to translate it to rads, derp
		if (wPressed)
		{
			eyePos += speed * framerate * vec3(sinf(tspin), cosf(tspin), 0.f);
		}
		if (sPressed)
		{
			eyePos -= speed * framerate * vec3(sinf(tspin), cosf(tspin), 0.f);
		}
		if (aPressed)
		{
			eyePos -= speed * framerate* vec3(cosf(tspin), -1 * sinf(tspin), 0.f);
		}
		if (dPressed)
		{
			eyePos += speed * framerate *vec3(cosf(tspin), -1 * sinf(tspin), 0.f);
		}


		eyePos.z = ground->HeightAt(eyePos.x, eyePos.y) + 2;
		WorldView = Rotate(0, tilt - 90.f) * Rotate(2, spin) * Translate(-1 * eyePos.x, -1*eyePos.y, -1*eyePos.z);
	}
	else
	{
		WorldView = Translate(tx, ty, -1 * zoom) * Rotate(0, tilt - 90.f) * Rotate(2, spin);
		//eyePos = WorldInverse * vec4(0, 0, 0, 1);
		

	}

	WorldProj = Perspective(rx, ry, front, back);

	/*
	WorldProj[0][0]=  2.368;
    WorldProj[0][1]= -0.800;
    WorldProj[0][2]=  0.000;
    WorldProj[0][3]=  0.000;
    WorldProj[1][0]=  0.384;
    WorldProj[1][1]=  1.136;
    WorldProj[1][2]=  2.194;
    WorldProj[1][3]=  0.000;
    WorldProj[2][0]=  0.281;
    WorldProj[2][1]=  0.831;
    WorldProj[2][2]= -0.480;
    WorldProj[2][3]= 42.451;
    WorldProj[3][0]=  0.281;
    WorldProj[3][1]=  0.831;
    WorldProj[3][2]= -0.480;
    WorldProj[3][3]= 43.442;
    
    WorldView[0][3]= -basePoint[0];
    WorldView[1][3]= -basePoint[1];
    WorldView[2][3]= -basePoint[2];
    */
    invert(&WorldView, &WorldInverse);
//	float dist = sqrtf(powf(lPos[0], 2) + powf(lPos[1], 2) + powf(lPos[2], 2));  //distance from light to center, center at 0,0,0



		LightView = LookAt(lPos[0], lPos[1], lPos[2], 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
		
		LightProj = Perspective((30.f/lightDist),(30.f/lightDist),0.1f, 1000.f);		//scene is approx [-40,40]x [-20,20]y -- might have that reversed though
		//Using the predefined lightDist of 1 million




		CHECKERROR;







		ShadowMatrix = Scale(0.5, 0.5, 0.5) * Translate(0.5, 0.5, 0.5) * LightProj * LightView;

		glViewport(0, 0, width, height);
		glClearColor(0.5, 0.5, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Start G Buffer 
		
		
		gBufferShader->Use();
		gBuffer->Bind();

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, width, height);
		glClearColor(0.5, 0.5, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		CHECKERROR;


		int programId = gBufferShader->programId;
		
		// Setup the perspective and viewing matrices for normal viewing.
		int loc;
		loc = glGetUniformLocation(programId, "WorldProj");
		glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());


		
		loc = glGetUniformLocation(programId, "WorldView");
		glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
		loc = glGetUniformLocation(programId, "WorldInverse");
		glUniformMatrix4fv(loc, 1, GL_TRUE, WorldInverse.Pntr());
		loc = glGetUniformLocation(programId, "lightPos");
		glUniform3fv(loc, 1, &(lPos[0]));
		//loc = glGetUniformLocation(programId, "mode");
		//glUniform1i(loc, mode);
		CHECKERROR;



		
		objectRoot->Draw(gBufferShader, Identity);

		for (std::vector<Object*>::iterator m = animated.begin(); m<animated.end(); m++)
			(*m)->animTr = Rotate(2, atime);

		CHECKERROR;
		
		
		/*
		
		
		loc = glGetUniformLocation(programId, "ShadowMatrix");
		glUniformMatrix4fv(loc, 1, GL_TRUE, ShadowMatrix.Pntr());


		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadowTexture->texture);
		loc = glGetUniformLocation(programId, "shadowTexture");
		glUniform1i(loc, 3);

		
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, reflectionTextureTop->texture);
		loc = glGetUniformLocation(programId, "reflectionTextureTop");
		glUniform1i(loc, 2);
		//CHECKERROR;


		
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, reflectionTextureBot->texture);
		loc = glGetUniformLocation(programId, "reflectionTextureBot");
		glUniform1i(loc, 4);
		//CHECKERROR;

		loc = glGetUniformLocation(programId, "tog");
		glUniform1f(loc, toggleReflection);
		*/

		HDRskydome->Bind(5);
		loc = glGetUniformLocation(programId, "skydomeTexture");
		glUniform1i(loc, 5);


		glActiveTexture(GL_TEXTURE13);
		glBindTexture(GL_TEXTURE_2D, parallaxMap->textureId);
		loc = glGetUniformLocation(programId, "parallaxMap");
		glUniform1i(loc, 13);

		glActiveTexture(GL_TEXTURE14);
		glBindTexture(GL_TEXTURE_2D, normalMap->textureId);
		loc = glGetUniformLocation(programId, "normalMap");
		glUniform1i(loc, 14);



		/*
		bricksTexture->Bind(6);
		loc = glGetUniformLocation(programId, "bricksTexture");
		glUniform1i(loc, 6);

		bricksNormalTexture->Bind(7);
		loc = glGetUniformLocation(programId, "normalMap");
		glUniform1i(loc, 7);

		*/
		
		

		gBuffer->Unbind();
		gBufferShader->Unuse();

		//End G Buffer
		CHECKERROR;

		











		
		
		
		
    // Use the lighting shader
		
		
		
		

		
		


		/*
		
		reflectionProgramTop->Use();
		reflectionTextureTop->Bind();
		CHECKERROR;

		glViewport(0, 0, 1024, 1024);
		// glViewport(0, 0, width, height);
		glClearColor(0.5, 0.5, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		CHECKERROR;
		int loc2, programId2;
		programId2 = reflectionProgramTop->programId;
		CHECKERROR;
		loc2 = glGetUniformLocation(programId2, "WorldProj");
		glUniformMatrix4fv(loc2, 1, GL_TRUE, WorldProj.Pntr());
		loc2 = glGetUniformLocation(programId2, "WorldView");
		glUniformMatrix4fv(loc2, 1, GL_TRUE, WorldView.Pntr());
		loc2 = glGetUniformLocation(programId2, "WorldInverse");
		glUniformMatrix4fv(loc2, 1, GL_TRUE, WorldInverse.Pntr());
		loc2 = glGetUniformLocation(programId2, "lightPos");
		glUniform3fv(loc2, 1, &(lPos[0]));
		loc2 = glGetUniformLocation(programId2, "mode");
		glUniform1i(loc2, mode);



		loc2 = glGetUniformLocation(programId2, "ShadowMatrix");
		glUniformMatrix4fv(loc2, 1, GL_TRUE, ShadowMatrix.Pntr());


		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, shadowTexture->texture);



		loc2 = glGetUniformLocation(programId2, "shadowTexture");
		glUniform1i(loc2, 2);


		skydome->Bind(5);
		loc2 = glGetUniformLocation(programId2, "skydomeTexture");
		glUniform1i(loc2, 5);

		bricksTexture->Bind(6);
		loc2 = glGetUniformLocation(programId2, "bricksTexture");
		glUniform1i(loc2, 6);

		bricksNormalTexture->Bind(7);
		loc2 = glGetUniformLocation(programId2, "normalMap");
		glUniform1i(loc2, 7);

		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		for (std::vector<Object*>::iterator m1 = animated.begin(); m1<animated.end(); m1++)
			(*m1)->animTr = Rotate(2, atime);
		
		// Draw all objects
		objectRootNoTeapot->Draw(reflectionProgramTop, Identity2);
		//glDisable(GL_CULL_FACE);
		reflectionTextureTop->Unbind();
		reflectionProgramTop->Unuse();
		
		
		
	//return;
	//	glBindTexture(GL_TEXTURE_2D, 0);
		
	
		

int loc3, programId3;

		

		
		reflectionProgramBot->Use();
		reflectionTextureBot->Bind();

		glViewport(0, 0, 1024, 1024);
		//	glViewport(0, 0, width, height);
		glClearColor(0.5, 0.5, 0.5, 1.0);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		CHECKERROR;

		programId3 = reflectionProgramBot->programId;

		loc3 = glGetUniformLocation(programId3, "WorldProj");
		glUniformMatrix4fv(loc3, 1, GL_TRUE, WorldProj.Pntr());
		loc3 = glGetUniformLocation(programId3, "WorldView");
		glUniformMatrix4fv(loc3, 1, GL_TRUE, WorldView.Pntr());
		loc3 = glGetUniformLocation(programId3, "WorldInverse");
		glUniformMatrix4fv(loc3, 1, GL_TRUE, WorldInverse.Pntr());
		loc3 = glGetUniformLocation(programId3, "lightPos");
		glUniform3fv(loc3, 1, &(lPos[0]));
		loc3 = glGetUniformLocation(programId3, "mode");
		glUniform1i(loc3, mode);


		loc3 = glGetUniformLocation(programId3, "ShadowMatrix");
		glUniformMatrix4fv(loc3, 1, GL_TRUE, ShadowMatrix.Pntr());
		CHECKERROR;

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, shadowTexture->texture);
		CHECKERROR;


		loc3 = glGetUniformLocation(programId3, "shadowTexture");
		glUniform1i(loc3, 2);
		CHECKERROR;

		skydome->Bind(5);
		loc3 = glGetUniformLocation(programId3, "skydomeTexture");
		glUniform1i(loc3, 5);

		bricksTexture->Bind(6);
		loc3 = glGetUniformLocation(programId3, "bricksTexture");
		glUniform1i(loc3, 6);

		bricksNormalTexture->Bind(7);
		loc3 = glGetUniformLocation(programId3, "normalMap");
		glUniform1i(loc3, 7);
		for (std::vector<Object*>::iterator m1 = animated.begin(); m1<animated.end(); m1++)
			(*m1)->animTr = Rotate(2, atime);

		// Draw all objects
		objectRootNoTeapot->Draw(reflectionProgramBot, Identity);
	
		reflectionTextureBot->Unbind();
		reflectionProgramBot->Unuse();
		
//	return;
	//	glBindTexture(GL_TEXTURE_2D, 0);

	*/
//glViewport(0, 0, width, height);
//glClearColor(0, 1, 0, 1.0);
//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//glClear(GL_COLOR_BUFFER_BIT);


//Ambient Occlusion pass
ambientOcclusionTexture->Bind();
ambientOcclusionShader->Use();


glViewport(0, 0, width, height);
glClear(GL_COLOR_BUFFER_BIT);
glDisable(GL_BLEND);
glDisable(GL_DEPTH_TEST);
programId = ambientOcclusionShader->programId;


glActiveTexture(GL_TEXTURE6);
glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[0]);
loc = glGetUniformLocation(programId, "gBuffer0");
glUniform1i(loc, 6);


glActiveTexture(GL_TEXTURE9);
glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[3]);
loc = glGetUniformLocation(programId, "gBuffer3");
glUniform1i(loc, 9);





loc = glGetUniformLocation(programId, "width");
glUniform1i(loc, width);

//		CHECKERROR;

loc = glGetUniformLocation(programId, "height");
glUniform1i(loc, height);
CHECKERROR;


loc = glGetUniformLocation(programId, "rangeOfInfluence");
glUniform1f(loc, rangeOfInfluence);

loc = glGetUniformLocation(programId, "NumSamples");
glUniform1i(loc, numSamples);


loc = glGetUniformLocation(programId, "width");
glUniform1i(loc, width);

//		CHECKERROR;

loc = glGetUniformLocation(programId, "height");
glUniform1i(loc, height);
CHECKERROR;


FSQ->Draw(ambientOcclusionShader, Identity);



ambientOcclusionShader->Unuse();
ambientOcclusionTexture->Unbind();




unsigned int blockID;
int programID1, loc1, bindpoint;
GLint imageUnit = 1;
bindpoint = 0;

//Ambient Occlusion Bilateral Blur


ambientOcclusionBilateralBlurShader->Use();
programId = ambientOcclusionBilateralBlurShader->programId;
//blockID++;
//GLuint blockID;
glGenBuffers(1, &blockID); // Generates block
						   //int	bindpoint = 3; // Start at zero, increment for other blocks

loc = glGetUniformBlockIndex(programId, "blurKernel");
glUniformBlockBinding(programId, loc, bindpoint);
glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, blockID);
glBufferData(GL_UNIFORM_BUFFER, kernelWidth * sizeof(float), (kernelWeights.data()), GL_STATIC_DRAW);
CHECKERROR;



loc = glGetUniformLocation(programId, "width");
glUniform1i(loc, width);

//		CHECKERROR;

loc = glGetUniformLocation(programId, "height");
glUniform1i(loc, height);
CHECKERROR;



loc = glGetUniformLocation(programId, "w");
glUniform1i(loc, kernelWidth / 2);
CHECKERROR;
imageUnit++;


loc = glGetUniformLocation(programId, "src");
glBindImageTexture(imageUnit, ambientOcclusionTexture->texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
glUniform1i(loc, imageUnit);
// Change GL_READ_ONLY to GL_WRITE_ONLY for output image
// Change GL_R32F to GL_RGBA32F for 4 channel images
CHECKERROR;
imageUnit++;
CHECKERROR;
loc = glGetUniformLocation(programId, "dst");
//	glBindImageTexture(imageUnit, shadowBlurPureTexture->textureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
glBindImageTexture(imageUnit, ambientOcclusionBlurredTexture->texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
glUniform1i(loc, imageUnit);
CHECKERROR;


glActiveTexture(GL_TEXTURE6);
glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[0]);
loc1 = glGetUniformLocation(programId, "gBuffer0");
glUniform1i(loc1, 6);




glActiveTexture(GL_TEXTURE9);
glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[3]);
loc1 = glGetUniformLocation(programId, "gBuffer3");
glUniform1i(loc1, 9);



glDispatchCompute(width / 128, height, 1);



ambientOcclusionBilateralBlurShader->Unuse();



bindpoint++;














		//Start Ambient Light G Buffer Pass

		//screenOutput->Bind();
glViewport(0, 0, width, height);
glClearColor(0.5, 0.5, 0.5, 1.0);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



			gBufferAmbientLighting->Use();
			glDisable(GL_BLEND);
			//glViewport(0, 0, width, height);
			//glClearColor(0.5, 0.5, 0.5, 1.0);
			//glClearDepth(1.0);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);   //Disable depth test?

			CHECKERROR;

			
			programId = gBufferAmbientLighting->programId;

			loc = glGetUniformLocation(programId, "ambient");
			glUniform3fv(loc, 1, &(ambientLight[0]));


			loc = glGetUniformLocation(programId, "WorldInverse");
			glUniformMatrix4fv(loc, 1, GL_TRUE, WorldInverse.Pntr());

			loc = glGetUniformLocation(programId, "AOSampleNum");
			glUniform1f(loc, AOSampleNum);

			loc = glGetUniformLocation(programId, "AOScale");
			glUniform1f(loc, AOScale);

			loc = glGetUniformLocation(programId, "AOContrast");
			glUniform1f(loc, AOContrast);





			glActiveTexture(GL_TEXTURE12);
			glBindTexture(GL_TEXTURE_2D, ambientOcclusionBlurredTexture->texture);
			//glBindTexture(GL_TEXTURE_2D, ambientOcclusionTexture->texture);
			loc = glGetUniformLocation(programId, "AOTexture");
			glUniform1i(loc, 12);

			HDRskydome->Bind(5);
			loc = glGetUniformLocation(programId, "skydomeTexture");
			glUniform1i(loc, 5);

			irradianceMap->Bind(10);
			loc = glGetUniformLocation(programId, "irradianceMap");
			glUniform1i(loc, 10);



			loc = glGetUniformLocation(programId, "exposure");
			glUniform1f(loc, exposure);


			loc = glGetUniformLocation(programId, "contrast");
			glUniform1f(loc, contrast);

			//Start 'pass gBuffer to specified shader' block

			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[0]);
			loc = glGetUniformLocation(programId, "gBuffer0");
			glUniform1i(loc, 6);




			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[1]);
			loc = glGetUniformLocation(programId, "gBuffer1");
			glUniform1i(loc, 7);




			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[2]);
			loc = glGetUniformLocation(programId, "gBuffer2");
			glUniform1i(loc, 8);



			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[3]);
			loc = glGetUniformLocation(programId, "gBuffer3");
			glUniform1i(loc, 9);



			loc = glGetUniformLocation(programId, "width");
			glUniform1i(loc, width);

			//		CHECKERROR;

			loc = glGetUniformLocation(programId, "height");
			glUniform1i(loc, height);
			CHECKERROR;


			loc = glGetUniformLocation(programId, "skyWidth");
			glUniform1i(loc, HDRskydome->HDRwidth);

			loc = glGetUniformLocation(programId, "skyHeight");
			glUniform1i(loc, HDRskydome->HDRheight);

			loc = glGetUniformLocation(programId, "HamN");
			int t = HamN;
			glUniform1i(loc, t);

			//End 'pass gBuffer to specified shader' block	

		
			
			

		//	unsigned int id1, bindpoint1;
		//	glGenBuffers(1, &id1);
		//	bindpoint1 = 4;
		//	glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint1, id1);
		//	glBufferData(GL_UNIFORM_BUFFER, sizeof(HamBlock), &HamBlock, GL_STATIC_DRAW);
			CHECKERROR;
		//	loc = glGetUniformBlockIndex(programId, "HammersleyBlock");
			CHECKERROR;
		//	glUniformBlockBinding(programId, loc, bindpoint1);
			CHECKERROR;
			FSQ->Draw(gBufferAmbientLighting,Identity);   //Maybe need projection transform to orient FSQ properly?

			gBufferAmbientLighting->Unuse();
		//	screenOutput->Unbind();


			//End Ambient Light G Buffer Pass
			


			//Start Shadow Depth test pass

			
			ShadowMatrix = Translate(0.5, 0.5, 0.5) * Scale(0.5, 0.5, 0.5) * LightProj * LightView;
			CHECKERROR;

			shadowProgram->Use();
			shadowTexture->Bind();

			CHECKERROR;


			glViewport(0, 0, 1024, 1024);
			glClearColor(0.5, 0.5, 0.5, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			CHECKERROR;
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);


			programID1 = shadowProgram->programId;
			loc1 = glGetUniformLocation(programID1, "ShadowProj");
			glUniformMatrix4fv(loc1, 1, GL_TRUE, LightProj.Pntr());
			//glUniformMatrix4fv(loc1, 1, GL_TRUE, WorldProj.Pntr());
			CHECKERROR

				loc1 = glGetUniformLocation(programID1, "ShadowView");
			glUniformMatrix4fv(loc1, 1, GL_TRUE, LightView.Pntr());
			//glUniformMatrix4fv(loc1, 1, GL_TRUE, WorldView.Pntr());

			CHECKERROR;
			loc1 = glGetUniformLocation(programID1, "WorldInverse");
			glUniformMatrix4fv(loc1, 1, GL_TRUE, WorldInverse.Pntr());


			//Start 'pass gBuffer to specified shader' block

			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[0]);
			loc1 = glGetUniformLocation(programID1, "gBuffer0");
			glUniform1i(loc1, 6);




			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[1]);
			loc1 = glGetUniformLocation(programID1, "gBuffer1");
			glUniform1i(loc1, 7);


			CHECKERROR;

			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[2]);
			loc1 = glGetUniformLocation(programID1, "gBuffer2");
			glUniform1i(loc1, 8);



			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[3]);
			loc1 = glGetUniformLocation(programID1, "gBuffer3");
			glUniform1i(loc1, 9);

			//End 'pass gBuffer to specified shader' block

			CHECKERROR;


			loc1 = glGetUniformLocation(programID1, "c");
			glUniform1i(loc1, shadowConstant);


			CHECKERROR;
			loc1 = glGetUniformLocation(programID1, "minDepth");
			glUniform1f(loc1, minDepth);


			loc1 = glGetUniformLocation(programID1, "maxDepth");
			glUniform1f(loc1, maxDepth);
			// Compute any continuously animating objects
			CHECKERROR;
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			for (std::vector<Object*>::iterator m1 = animated.begin(); m1<animated.end(); m1++)
				(*m1)->animTr = Rotate(2, atime);

			// Draw all objects
			objectRootNoTeapot->Draw(shadowProgram, Identity1);
			glDisable(GL_CULL_FACE);

			shadowTexture->Unbind();
			shadowProgram->Unuse();

			//End Shadow Depth test pass
			
			CHECKERROR;


			shadowBlurComputeShader->Use();
			programId = shadowBlurComputeShader->programId;
			
			
		//	shadowBlurPureTexture->Bind();
			CHECKERROR;

			//GLuint blockID;
			glGenBuffers(1, &blockID); // Generates block
			//int	bindpoint = 0; // Start at zero, increment for other blocks
				loc = glGetUniformBlockIndex(programId, "blurKernel");
				glUniformBlockBinding(programId, loc, bindpoint);
				glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, blockID);
				glBufferData(GL_UNIFORM_BUFFER, kernelWidth * sizeof(float), (kernelWeights.data()), GL_STATIC_DRAW);
				CHECKERROR;


			  //Increment for other images
				loc = glGetUniformLocation(programId, "src");
				glBindImageTexture(imageUnit, shadowTexture->texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
				glUniform1i(loc, imageUnit);
					// Change GL_READ_ONLY to GL_WRITE_ONLY for output image
					// Change GL_R32F to GL_RGBA32F for 4 channel images
				CHECKERROR;
				imageUnit++;
				CHECKERROR;
				loc = glGetUniformLocation(programId, "dst");
			//	glBindImageTexture(imageUnit, shadowBlurPureTexture->textureId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				glBindImageTexture(imageUnit, blurredShadowTexture->texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				glUniform1i(loc, imageUnit);
				CHECKERROR;


				loc = glGetUniformLocation(programId, "w");
				glUniform1i(loc, kernelWidth / 2);
				CHECKERROR;
				loc = glGetUniformLocation(programId, "minDepth");
				glUniform1f(loc, minDepth);
				CHECKERROR;

				loc = glGetUniformLocation(programId, "maxDepth");
				glUniform1f(loc, maxDepth);
				CHECKERROR;

				loc = glGetUniformLocation(programId, "c");
				glUniform1i(loc, shadowConstant);
				CHECKERROR;

			glDispatchCompute(shadowTextureWidth / 128, shadowTextureHeight, 1); // Tiles WxH image with groups sized 128x1
		//	shadowTexture->Unbind();
			CHECKERROR;
			shadowBlurComputeShader->Unuse();
			CHECKERROR;
















			//Start Global (Shadow-casting) Light G Buffer Pass
			
			
	//		screenOutput->Bind();

			glViewport(0, 0, width, height);
		//	glClearColor(0.5, 0.5, 0.5, 1.0);
		//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			CHECKERROR;

			gBufferGlobalLighting->Use();

			glDisable(GL_DEPTH_TEST);
			CHECKERROR;
			glEnable(GL_BLEND);
			CHECKERROR;
			 programId = gBufferGlobalLighting->programId;
		

			loc = glGetUniformLocation(programId, "ShadowMatrix");
			glUniformMatrix4fv(loc, 1, GL_TRUE, ShadowMatrix.Pntr());


			glActiveTexture(GL_TEXTURE3);
		//	glBindTexture(GL_TEXTURE_2D, shadowTexture->texture);
		//	glBindTexture(GL_TEXTURE_2D, shadowBlurPureTexture->textureId);
			glBindTexture(GL_TEXTURE_2D, blurredShadowTexture->texture);
			CHECKERROR;
			
			loc = glGetUniformLocation(programId, "shadowTexture");
			glUniform1i(loc, 3);

			loc = glGetUniformLocation(programId, "WorldInverse");
			glUniformMatrix4fv(loc, 1, GL_TRUE, WorldInverse.Pntr());


			//Start 'pass gBuffer to specified shader' block
		
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[0]);
			loc1 = glGetUniformLocation(programId, "gBuffer0");
			glUniform1i(loc1, 6);




			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[1]);
			loc1 = glGetUniformLocation(programId, "gBuffer1");
			glUniform1i(loc1, 7);




			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[2]);
			loc1 = glGetUniformLocation(programId, "gBuffer2");
			glUniform1i(loc1, 8);



			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[3]);
			loc1 = glGetUniformLocation(programId , "gBuffer3");
			glUniform1i(loc1, 9);


			CHECKERROR;

			loc1 = glGetUniformLocation(programId, "Light");
			glUniform3fv(loc1, 1, &(lightColor[0]));

			loc1 = glGetUniformLocation(programId, "lightPos");
			glUniform3fv(loc1,1,&(lPos[0]));

			CHECKERROR;


			loc1 = glGetUniformLocation(programId, "width");
			glUniform1i(loc1, width);

	//		CHECKERROR;

			loc1 = glGetUniformLocation(programId, "height");
			glUniform1i(loc1,height);
	//		CHECKERROR;

			loc1 = glGetUniformLocation(programId, "minDepth");
			glUniform1f(loc1, minDepth);
			CHECKERROR;

			loc1 = glGetUniformLocation(programId, "maxDepth");
			glUniform1f(loc1, maxDepth);
			CHECKERROR;

			loc1 = glGetUniformLocation(programId, "c");
			glUniform1i(loc1, shadowConstant);

			//End 'pass gBuffer to specified shader' block
			FSQ->Draw(gBufferGlobalLighting, Identity);
			CHECKERROR;
			gBufferGlobalLighting->Unuse();
	//		screenOutput->Unbind();
			


			//End Global (Shadow-casting) Light G Buffer Pass

		
			




			
			//Start local lighting (small lights with pre-defined radii) pass
	//		screenOutput->Bind();
			glViewport(0, 0, width, height);
		//	glClearColor(0.5, 0.5, 0.5, 1.0);
		//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
			
			/*

			gBufferLocalLighting->Use();
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			CHECKERROR;

			programId = gBufferLocalLighting->programId;
			
			loc = glGetUniformLocation(programId, "WorldProj");
			glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
			loc = glGetUniformLocation(programId, "WorldView");
			glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
			loc = glGetUniformLocation(programId, "WorldInverse");
			glUniformMatrix4fv(loc, 1, GL_TRUE, WorldInverse.Pntr());
			
			loc = glGetUniformLocation(programId, "radius");
			glUniform1f(loc, localLightRadius);

			
			//Start 'pass gBuffer to specified shader' block

			


			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[0]);
			loc1 = glGetUniformLocation(programId, "gBuffer0");
			glUniform1i(loc1, 6);




			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[1]);
			loc1 = glGetUniformLocation(programId, "gBuffer1");
			glUniform1i(loc1, 7);




			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[2]);
			loc1 = glGetUniformLocation(programId, "gBuffer2");
			glUniform1i(loc1, 8);




			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[3]);
			loc1 = glGetUniformLocation(programId, "gBuffer3");
			glUniform1i(loc1, 9);

			loc1 = glGetUniformLocation(programId, "radius");
			glUniform1f(loc1, localLightRadius);

			loc1 = glGetUniformLocation(programId, "radiusSquared");
			glUniform1f(loc1, localLightRadius*localLightRadius);

			loc1 = glGetUniformLocation(programId, "localLightBrightness");
			glUniform3fv(loc1, 1, &localLightColor[0]);



			loc1 = glGetUniformLocation(programId, "width");
			glUniform1i(loc1, width);

			//		CHECKERROR;

			loc1 = glGetUniformLocation(programId, "height");
			glUniform1i(loc1, height);
 				CHECKERROR;

			//End 'pass gBuffer to specified shader' block
		
		//	
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			localLights->DrawLights(gBufferLocalLighting, Identity);

			glDisable(GL_CULL_FACE);
			CHECKERROR;
		//	FSQ->Draw(gBufferLocalLighting, Identity);
			CHECKERROR;
			gBufferLocalLighting->Unuse();
	//		screenOutput->Unbind();

			CHECKERROR;


			//End local lighting pass
			
			glDisable(GL_BLEND);
*/

/*

int programId, loc;
			
    lightingProgram->Use();

	glViewport(0, 0, width, height);
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



     programId = lightingProgram->programId;

    // Setup the perspective and viewing matrices for normal viewing.
  
    loc = glGetUniformLocation(programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
    loc = glGetUniformLocation(programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
    loc = glGetUniformLocation(programId, "WorldInverse");
    glUniformMatrix4fv(loc, 1, GL_TRUE, WorldInverse.Pntr());
    loc = glGetUniformLocation(programId, "lightPos");
    glUniform3fv(loc, 1, &(lPos[0]));  
    //loc = glGetUniformLocation(programId, "mode");
    //glUniform1i(loc, mode);  
	
	loc = glGetUniformLocation(programId, "ShadowMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, ShadowMatrix.Pntr());
	

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, shadowTexture->texture);
	loc = glGetUniformLocation(programId, "shadowTexture");
	glUniform1i(loc, 3);
	
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, reflectionTextureTop->texture);
	loc = glGetUniformLocation(programId, "reflectionTextureTop");
	glUniform1i(loc, 2);
	//CHECKERROR;



	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, reflectionTextureBot->texture);
	loc = glGetUniformLocation(programId, "reflectionTextureBot");
	glUniform1i(loc, 4);
	//CHECKERROR;

	loc = glGetUniformLocation(programId, "tog");
		glUniform1f(loc, toggleReflection);

	
		skydome->Bind(5);
		loc = glGetUniformLocation(programId, "skydomeTexture");
		glUniform1i(loc, 5);

		bricksTexture->Bind(6);
		loc = glGetUniformLocation(programId, "bricksTexture");
		glUniform1i(loc, 6);

		bricksNormalTexture->Bind(7);
		loc = glGetUniformLocation(programId, "normalMap");
		glUniform1i(loc, 7);


	
	//gluLookAt

	//MAT4  NormalInverse;
	//MAT4 NormalTransform(objectRoot->shape->modelTr);
//	invert(&NormalInverse, &objectRoot->shape->modelTr);

	//loc = glGetUniformLocation(programId, "NormalTr");
	//glUniformMatrix4fv(loc, 1, GL_TRUE, (objectRoot->shape->modelTr).Pntr());


	//SH added values
	//loc = glGetUniformLocation(programId, "eyePos");
//	vec3 eye = (WorldInverse*glm::vec4(0, 0, 0, 1)).xyz;
	//glUniform3fv(loc,1, &eye[0]);



	//Object properties - will probablly need to tweak these / add in another variable to tell what object is what so I can apply the texture to the sphere properly
	//loc = glGetUniformLocation(programId, "diffuseColor"); 
	//glUniform3fv(loc, 1, &(diffuseColor[0])));
	//loc = glGetUniformLocation(programId, "specularColor");
	//glUniform3fv(loc, 1, &(objectRoot->shape->specularColor[0]));
	//loc = glGetUniformLocation(programId, "shininess");
	//glUniform1f(loc, (objectRoot->shape)->shininess);
	//CHECKERROR;

	//Light values



    CHECKERROR;

    // Compute any continuously animating objects
    for (std::vector<Object*>::iterator m=animated.begin();  m<animated.end();  m++)
        (*m)->animTr = Rotate(2,atime);

    // Draw all objects
    objectRoot->Draw(lightingProgram, Identity);

    lightingProgram->Unuse();
	
//	glBindTexture(GL_TEXTURE_2D, 0);
*/
	
//int loc;
/*
basicOutputShader->Use();
CHECKERROR;

glViewport(0, 0, width, height);
glClearColor(0.5, 0.5, 0.5, 1.0);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
CHECKERROR;
programId = basicOutputShader->programId;
glActiveTexture(GL_TEXTURE14);
CHECKERROR;
glBindTexture(GL_TEXTURE_2D, gBuffer->renderTargets[1]);
CHECKERROR;
loc = glGetUniformLocation(programId, "screenOutputTexture");
glUniform1i(loc, 14);
CHECKERROR;

loc1 = glGetUniformLocation(programId, "width");
glUniform1i(loc1, width);

//		CHECKERROR;

loc1 = glGetUniformLocation(programId, "height");
glUniform1i(loc1, height);
		CHECKERROR;
FSQ->Draw(basicOutputShader, Identity);
CHECKERROR;
/*
for (std::vector<Object*>::iterator m = animated.begin(); m<animated.end(); m++)
(*m)->animTr = Rotate(2, atime);

objectRoot->Draw(basicOutputShader,Identity);



basicOutputShader->Unuse();


	
*/
	prevTime = curTime;
	curTime = glutGet((GLenum)GLUT_ELAPSED_TIME);

    CHECKERROR;

}
