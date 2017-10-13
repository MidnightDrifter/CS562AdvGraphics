////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw the (really) simple scene, including:
//   * Geometry (in a display list)
//   * Light parameters
//   * Material properties
//   * Viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them should be used to draw the scene.

#include "shapes.h"
#include "object.h"
#include "texture.h"
#include "fbo.h"


enum ObjectIds {
    nullId	= 0,
    skyId	= 1,
    seaId	= 2,
    groundId	= 3,
    wallId	= 4,
    boxId	= 5,
    frameId	= 6,
    lPicId	= 7,
    rPicId	= 8,
    teapotId	= 9,
    spheresId	= 10,
	localLightsId=11,
};

class Shader;

class Scene
{
public:
    // Viewing transformation parameters (suggested) FIXME: This is a
    // good place for the transformation values which are set by the
    // user mouse/keyboard actions and used in DrawScene to create the
    // transformation matrices.

	const float PI = 3.14159f;
    ProceduralGround* ground;

    // Light position parameters
    float lightSpin, lightTilt, lightDist;

    vec3 basePoint;  // Records where the scene building is centered.
    int mode; // Extra mode indicator hooked up to number keys and sent to shader
    
    // Viewport
   // int width, height;
	int width = 1024, height = 1024;
	float speed = 10.0f;
	bool isToggled = false;
	
	vec3 eyePos = vec3(0.f, 0.f, 0.f);

	bool wPressed = false;
	bool aPressed = false;
	bool sPressed = false;
	bool dPressed = false;

	float prevTime, curTime;


	float spin = 0.f;  //Left-right mouse movements
	float tilt = -90.f;  //Up-down mouse movements

	float tx = 0.f;  //Translate horizontal - screen dragging
	float ty = 0.f;  //Translate vertical - screen dragging

	float zoom = 150.f; //Zoom in/out via scroll wheel

	float ry = 0.2f;  //Y-slope for perspective transform
	float rx = 0;  //X-slope for  perspective transform
	float front = 0.1f;  //Front clipping plane for perspective transform
	float back = 3000.f; //Back clipping plane for perspective transform
	float toggleReflection = 1.f;  //float to toggle regular lighting vs. reflection-only in shaders

	vec3 ambientLight = vec3(0.02f*PI, PI*0.02f, PI* 0.02f);  //Float to denote the amount of ambient light - to be used until Proj 3
		

    // All objects in the scene are children of this single root object.
    Object* objectRoot;
	Object* objectRootNoTeapot;
    std::vector<Object*> animated;
	Object* localLights;
	int numLocalLights = 250;
	float localLightRadius = 15.f;  //Likely assume that, for each light, their sphere of influence is approx. their radius or some multiple of it
	int shadowConstant = 60;  //C value for exponential shadow map
	vec3 lightColor = vec3(PI, PI, PI);
	vec3 localLightColor = vec3(PI , PI/8 ,PI/8  ); /// vec3(numLocalLights, numLocalLights, numLocalLights);
	  //So much reorganizing!
									
	// Shader programs
    ShaderProgram* lightingProgram, *shadowProgram, *reflectionProgramTop, *reflectionProgramBot, *gBufferShader, *gBufferGlobalLighting, *gBufferAmbientLighting, *gBufferLocalLighting, *basicOutputShader, *shadowBlurComputeShader;

	//FBOs
	FBO* shadowTexture,* reflectionTextureTop,* reflectionTextureBot, *gBuffer, *screenOutput, *blurredShadowTexture;

	//Textures - testin'
	Texture* test;
	Texture *bricksNormalTexture,* bricksTexture, *skydome;

	const float e = 2.7182818284590452353602874713527;
	const static int kernelWidth = 25;
	const float s = kernelWidth / 2.f;
	float kernelWeights[kernelWidth];// = new int[kernelWidth];
	


	float minDepth = 0.5f;

	float maxDepth = 50.f;



	Object* FSQ; //Full screen quad

    //void append(Object* m) { objects.push_back(m); }

    void InitializeScene();
    void DrawScene();

};
