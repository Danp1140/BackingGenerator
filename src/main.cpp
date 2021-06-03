#include "Viewport.h"

int main(){
	Viewport::initGL();
	Viewport v=Viewport();

	do{
		v.draw();
	}while(!glfwWindowShouldClose(v.window)&&glfwGetKey(v.window, GLFW_KEY_ESCAPE)!=GLFW_PRESS);

	return 0;
}