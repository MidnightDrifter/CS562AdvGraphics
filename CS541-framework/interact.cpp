
#include "framework.h"

extern Scene scene;       // Declared in framework.cpp, but used here.

// Some globals used for mouse handling.
int mouseX, mouseY;
bool shifted = false;
bool leftDown = false;
bool middleDown = false;
bool rightDown = false;

////////////////////////////////////////////////////////////////////////
// Called by GLUT when the scene needs to be redrawn.
void ReDraw()
{
    scene.DrawScene();
    glutSwapBuffers();
}

////////////////////////////////////////////////////////////////////////
// Function called to exit
void Quit(void *clientData)
{
    glutLeaveMainLoop();
}

////////////////////////////////////////////////////////////////////////
// Called by GLUT when the window size is changed.
void ReshapeWindow(int w, int h)
{
    if (w && h) {
        glViewport(0, 0, w, h); }
    scene.width = w;
    scene.height = h;

	scene.rx = scene.ry * (w * 1.f / h);

//NEW:  re-make G-Buffer with new width & height
	if (scene.gBuffer)
	{
	//	delete scene.gBuffer;
	//	scene.gBuffer = new FBO();
//		scene.gBuffer->Unbind();
		scene.gBuffer->CreateGBuffer(w, h);
		
	}

	if (scene.ambientOcclusionTexture)
	{
		scene.ambientOcclusionTexture->CreateFBO(w, h);
	}

	if (scene.ambientOcclusionBlurredTexture)
	{
		scene.ambientOcclusionBlurredTexture->CreateFBO(w, h);
	}


    // Force a redraw
    glutPostRedisplay();
}

////////////////////////////////////////////////////////////////////////
// Called by GLut for keyboard actions.
void KeyboardDown(unsigned char key, int x, int y)
{
    printf("key down %c(%d)\n", key, key);
    fflush(stdout);
  
	switch (key) {
	case 27: case 'q':       // Escape and 'q' keys quit the application
		exit(0);
	case 't':
		scene.isToggled = !scene.isToggled;
	case 'r':
		scene.toggleReflection *= -1.f;
	}
	if (scene.isToggled)
		{
		if (key == 'w')
		{
			scene.wPressed = true;
		}
		//scene.eyePos += scene.speed * vec3(sinf(scene.spin), cosf(scene.spin), 0.f);
		if (key == 's')
			//scene.eyePos -= scene.speed * vec3(sinf(scene.spin), cosf(scene.spin), 0.f);
		{
			scene.sPressed = true;
		}
		if (key == 'a')
		{
			scene.aPressed = true;
		}
			//scene.eyePos -= scene.speed * vec3(cosf(scene.spin), -1 * sinf(scene.spin), 0.f);
		if (key == 'd')
		{
			scene.dPressed = true;
		}
		//scene.eyePos += scene.speed*vec3(cosf(scene.spin), -1 * sinf(scene.spin), 0.f);





    }
}

void KeyboardUp(unsigned char key, int x, int y)
{
	if (key == 'a')
		scene.aPressed = false;
	else if (key == 's')
		scene.sPressed = false;
	else if (key == 'd')
		scene.dPressed = false;
	else if (key == 'w')
		scene.wPressed = false;
	
	fflush(stdout);
}

////////////////////////////////////////////////////////////////////////
// Called by GLut when a mouse button changes state.
void MouseButton(int button, int state, int x, int y)
{        
    
    // Record the position of the mouse click.
    mouseX = x;
    mouseY = y;

    // Test if the SHIFT key was down for this mouse click
    shifted = glutGetModifiers() & GLUT_ACTIVE_SHIFT;

    // Ignore high order bits, set by some (stupid) GLUT implementation.
    button = button%8;

    // Figure out the mouse action, and handle accordingly
    if (button == 3 && shifted) { // Scroll light in
        scene.lightDist = pow(scene.lightDist, 1.0f/1.02f);
        printf("shifted scroll up\n"); }

    else if (button == 4 && shifted) { // Scroll light out
        scene.lightDist = pow(scene.lightDist, 1.02f);
        printf("shifted scroll down\n"); }

    else if (button == GLUT_LEFT_BUTTON) {
        leftDown = (state == GLUT_DOWN);
        printf("Left button down\n");
	
	
	}

    else if (button == GLUT_MIDDLE_BUTTON) {
        middleDown = (state == GLUT_DOWN);
        printf("Middle button down\n");  }

    else if (button == GLUT_RIGHT_BUTTON) {
        rightDown = (state == GLUT_DOWN);
        printf("Right button down\n");  }

    else if (button == 3) {
        printf("scroll up\n");
		scene.zoom *= 1.025f;
	
	}

    else if (button == 4) {
        printf("scroll down\n"); 
		scene.zoom /= 1.025f;
	}


    // Force a redraw
    glutPostRedisplay();
    fflush(stdout);
}

////////////////////////////////////////////////////////////////////////
// Called by GLut when a mouse moves (while a button is down)
void MouseMotion(int x, int y)
{
    // Calculate the change in the mouse position
    int dx = x-mouseX;
    int dy = y-mouseY;

    if (leftDown && shifted) {  // Rotate light position
        scene.lightSpin += dx/3.0;
        scene.lightTilt -= dy/3.0; }

    else if (leftDown) {
		scene.spin += dx / 10.f;  //Arbitrarily chosen value for rotating, can tweak 
		scene.tilt += dy / 10.f;
	
	}

    if (middleDown && shifted) {
        scene.lightDist = pow(scene.lightDist, 1.0f-dy/200.0f);  }

    else if (middleDown) { }

    if (rightDown) {

		scene.tx += dx / 50.f;
		scene.ty += dy / 50.f;
    }

    // Record this position
    mouseX = x;
    mouseY = y;

    // Force a redraw
    glutPostRedisplay();
}

void InitInteraction()
{
    glutIgnoreKeyRepeat(true);
    
    glutDisplayFunc(&ReDraw);
    glutReshapeFunc(&ReshapeWindow);

    glutKeyboardFunc(&KeyboardDown);
    glutKeyboardUpFunc(&KeyboardUp);

    glutMouseFunc(&MouseButton);
    glutMotionFunc(&MouseMotion);
}
