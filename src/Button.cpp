//
// Created by Daniel Paavola on 2020-05-02.
//

#include "Button.h"

int Button::nextid=0;
GLuint Button::buttonshaders=-1;

Button::Button(){
//	position=glm::vec2(0.0f, 0.0f);
//	color=glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
//	const GLFWvidmode*mode=glfwGetVideoMode(glfwGetPrimaryMonitor());
//	displaywidth=mode->width;
//	displayheight=mode->height;
//	message=Text();
//	if(buttonshaders==-1) buttonshaders=loadShaders(BUTTON_VERTEX_SHADER_PATH, BUTTON_FRAGMENT_SHADER_PATH);
//	shaders=buttonshaders;
//	autoScale();
//	id=nextid;
//	nextid++;
}

Button::Button(std::wstring t, glm::vec2 p, glm::vec4 c, int f, ButtonType bt):color(c), type(bt){
	position=p;
	const GLFWvidmode*mode=glfwGetVideoMode(glfwGetPrimaryMonitor());
	displaywidth=mode->width;
	displayheight=mode->height;
	message=Text(t, p, glm::vec2(0, 0), f, glm::vec4(0, 0, 0, 1));
	if(buttonshaders==-1) buttonshaders=loadShaders(BUTTON_VERTEX_SHADER_PATH, BUTTON_FRAGMENT_SHADER_PATH);
	shaders=buttonshaders;
	autoScale();
	id=nextid;
	nextid++;
}

Button::~Button(){
	glDeleteBuffers(1, &vertexbuffer);
}

void Button::refreshVertices(){
	vertices=std::vector<glm::vec2>(0);
	vertices.push_back(position);
	vertices.push_back(position+glm::vec2(scale.x, 0.0f));
	vertices.push_back(position+glm::vec2(0.0f, scale.y));
	vertices.push_back(position+glm::vec2(scale.x, 0.0f));
	vertices.push_back(position+glm::vec2(0.0f, scale.y));
	vertices.push_back(position+glm::vec2(scale.x, scale.y));
	message.setPosition(position);
}

void Button::draw(){
	glUseProgram(shaders);
	GLint widthloc=glGetUniformLocation(shaders, "width");
	GLint heightloc=glGetUniformLocation(shaders, "height");
	GLint colorloc=glGetUniformLocation(shaders, "colorin");
	glUniform1i(widthloc, displaywidth);
	glUniform1i(heightloc, displayheight);
	glUniform4fv(colorloc, 1, &color[0]);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
	glDisableVertexAttribArray(0);
	glDeleteBuffers(1, &vertexbuffer);

	message.draw();
}

void Button::autoScale(){
	float maxx=0, maxy=0;
	for(auto&v:message.getVertices()){
		if(v.x-message.getPosition().x>maxx){maxx=v.x-message.getPosition().x;}
		if(v.y-message.getPosition().y>maxy){maxy=v.y-message.getPosition().y;}
	}
	scale=glm::vec2(maxx, maxy);
	refreshVertices();
}

void Button::setPosition(glm::vec2 p){
	position=p;
	refreshVertices();
}

void Button::setScale(glm::vec2 s){
	scale=s;
	refreshVertices();
}

bool Button::operator<(const Button&b) const{
	return this->id<b.id;
}