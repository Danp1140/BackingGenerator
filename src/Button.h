//
// Created by Daniel Paavola on 2020-05-02.
//

#ifndef BACKINGGENERATOR_BUTTON_H
#define BACKINGGENERATOR_BUTTON_H

#include "Drawable.h"
#include "Text.h"

#define BUTTON_VERTEX_SHADER_PATH "../resources/shaders/ButtonVertexShader.glsl"
#define BUTTON_FRAGMENT_SHADER_PATH "../resources/shaders/ButtonFragmentShader.glsl"

enum ButtonType {
	BUTTON_TEXT_EDITOR,
	BUTTON_SLASH
};

class Button:public Drawable{
private:
	static int nextid;
	ButtonType type;
	//Recalculates button's vertices, and calls message's refreshVertices via a setPosition call
	void refreshVertices();
public:
	static GLuint buttonshaders;
	int id;
	Text message;
	glm::vec4 color;
	//Experimental callback function, not necisarilly final
	//if BUTTON_TEXT_EDITOR: bool callback(ButtonType bt, Button*edit, int*input)
	//input is array of wchar_t codepoints
	//if BUTTON_SLASH: bool callback(ButtonType bt, Chord*edit, int*input)
	bool(*callback)(ButtonType...);

	Button();
	//Constructs a button with given position and color, as well as constructing a text object for message with the
	//appropriate position and text
	Button(std::wstring t, glm::vec2 p, glm::vec4 c, int f, ButtonType bt);

	~Button();
	//Draws button, and text object stored in button
	void draw();
	//Sets the scale of the button to be as wide as the text, currently only sets height to that of one line of text,
	//which should be fixed
	void autoScale();
	//Sets button's position variable, and refreshes button's vertices
	void setPosition(glm::vec2 p);

	void setScale(glm::vec2 s);

	Text*getMessage(){return &message;}

	ButtonType getType(){return type;}

	bool operator<(const Button&b) const;
};


#endif //BACKINGGENERATOR_BUTTON_H
