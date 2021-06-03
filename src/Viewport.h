//
// Created by Daniel Paavola on 2020-04-30.
//

#ifndef BACKINGGENERATOR_VIEWPORT_H
#define BACKINGGENERATOR_VIEWPORT_H

#include "Button.h"
#include "Song.h"
#include "AudioHandler.h"
#include <chrono>

typedef struct{
	std::wstring textinputbuffer;
	bool mousepress=false, lastmousepress=false;
} inputdata;

class Viewport{
private:
	Button*activebutton;
	//ah must be pointer for unknown reason, diagnose later
	AudioHandler*ah;
	Song*song;
	GLuint vertexarray;
	Button masterpeakmeterright, masterpeakmeterleft;
	Text vstparams;
	//Callback function called by glfw every time character input is generated by the keyboard. Adds character input
	//to callbackdata's textinputbuffer
	static void charStreamCallback(GLFWwindow*window, unsigned int codepoint);
	//Handles key input not used in charStreamCallback. Deletes one char from end of textinputbuffer when backspace
	//is pressed. Adds '\n' to end of textinputbuffer when enter is pressed.``
	static void keyboarGdCallback(GLFWwindow*window, int key, int scancode, int action, int mods);
	//Sets boolean value of mousepress appropriately, in order to facilitate button class functionality
	static void mouseButtonCallback(GLFWwindow*window, int button, int action, int mods);
	//Experimental callback function, not necessarily final
	static bool buttonCallback(ButtonType bt, ...);
public:
	static GLFWwindow*window;
	static int width, height;
	Text*times;
	inputdata*callbackdata;

	//Constructor handles initialization of OpenGL libraries, as well as instantiation of a window, and other important
	//viewport variables
	Viewport();

	//Destructor handles termination and destruction of OpenGL libraries, objects, and buffers.
	~Viewport();

	static void initGL();
	//Draws each drawable stored in the Viewport, as well as calling essential OpenGL window functions
	void draw();
};

#endif //BACKINGGENERATOR_VIEWPORT_H
