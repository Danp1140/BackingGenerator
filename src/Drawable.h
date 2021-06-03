//
// Created by Daniel Paavola on 2020-04-30.
//

#ifndef BACKINGGENERATOR_DRAWABLE_H
#define BACKINGGENERATOR_DRAWABLE_H

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

class Drawable{
private:

protected:
	glm::vec2 position, scale;
	GLuint shaders, vertexbuffer;
	std::vector<glm::vec2> vertices;
	int displaywidth, displayheight;
public:
	//Implementations draw whatever may be represented by the Drawable
	virtual void draw()=0;
	//Returns vec2 of drawable's position (the bottom left)
	glm::vec2 getPosition(){return position;}
	//Implementation sets position variable and resets vertices of object, as well as calling the setPosition function
	//of other drawables owned
	virtual void setPosition(glm::vec2 p)=0;

	glm::vec2 getScale(){return scale;}
	//Returns the vector of vec2 vertices of the drawable
	std::vector<glm::vec2> getVertices(){return vertices;}
	//Loads vertex shader and fragment shaders from GLSL files
	static GLuint loadShaders(const char*vertexshaderpath, const char*fragmentshaderpath);
};


#endif //BACKINGGENERATOR_DRAWABLE_H
