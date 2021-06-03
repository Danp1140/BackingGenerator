//
// Created by Daniel Paavola on 2020-04-30.
//

#ifndef BACKINGGENERATOR_TEXT_H
#define BACKINGGENERATOR_TEXT_H

#include <string>
#include <ft2build.h>
#include <map>
#include FT_FREETYPE_H
#include "Drawable.h"

#define TEXT_VERTEX_SHADER_PATH "../resources/shaders/TextVertexShader.glsl"
#define TEXT_FRAGMENT_SHADER_PATH "../resources/shaders/TextFragmentShader.glsl"

#define TEXT_SCALING_FACTOR 512
#define MAX_TEXT_LENGTH 512

class Text:public Drawable{
private:
	static constexpr glm::vec2 uvs[6]={glm::vec2(1, 0), glm::vec2(0, 0), glm::vec2(0, 1), glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0)};
	static GLuint*chartextures;
	static std::map<wchar_t, GLuint> outofrangechartextures;
	//delta may not be used
	static constexpr wchar_t outofrangecharstoload[11]={0xe10c, 0xe10d, 0xe188, 0xe189, 0xe18a, 0xe191, 0xe193, 0xe195, 0xe196, 0xe197, 0xe199};
	//Recalculates the position of vertices according to position and scale variables, as well as freetype parameters
	void refreshVertices();

	void loadCharTextures();
public:
	//really shitty name
	static GLuint textshaders;
	std::wstring text;
	FT_Face face;
	FT_Library lib;
	int fontsize;
	glm::vec4 fontcolor;
	GLuint uvbuffer;
	std::vector<GLuint> textures;

	//Default constructor for button class. Initializes with somewhat arbitrary values
	Text();
	//Constructor handles Freetype library initaialization, as well as setting essential variable values
	Text(std::wstring t, glm::vec2 p, glm::vec2 s, int fs, glm::vec4 c);

	~Text();
	//Draws text in appropriate color and size using provided shaders. Scale is notably ignored
	void draw();
	//Sets the position of the text, updating vertices
	void setPosition(glm::vec2 p);
	//Sets the string text, updating vertices
	void setText(std::wstring t);
	//Returns int of font size
	int getFontSize(){return fontsize;}
	//Sets the fontsize variable, updates the freetype library's font size, and refreshes vertices
	void setFontSize(int fs);

	std::wstring getText(){return text;}
};


#endif //BACKINGGENERATOR_TEXT_H
