//
// Created by Daniel Paavola on 2020-05-25.
//

#ifndef BACKINGGENERATOR_CHORD_H
#define BACKINGGENERATOR_CHORD_H

#include "Button.h"

//consider making enum for root notes

class Chord:public Drawable{
private:
	//start and length may be unnecessary
	std::wstring root, formatted;
	Text text;
	//numbers will only vary by 0.5
	std::vector<float> notes;
	std::wstring parse(std::wstring s);
public:

	Chord();

	Chord(std::wstring s, glm::vec2 p);

	void draw();

	void setPosition(glm::vec2 p);

	std::wstring getFormatted(){return formatted;}

	std::wstring getRoot(){return root;}

	void setTextRaw(std::wstring t);

	Text*getText(){return &text;}

	std::vector<float> getNotes(){return notes;}

};


#endif //BACKINGGENERATOR_CHORD_H
