//
// Created by Daniel Paavola on 2020-05-25.
//

#include "Chord.h"

Chord::Chord(){
	position=glm::vec2(0, 0);
	scale=glm::vec2(0, 0);
	shaders=0;
	vertexbuffer=0;
	vertices=std::vector<glm::vec2>(0);
	const GLFWvidmode*mode=glfwGetVideoMode(glfwGetPrimaryMonitor());
	displaywidth=mode->width; displayheight=mode->height;
	notes={};
	root=L"";
	text=Text(L"", position, scale, 2, glm::vec4(0, 0, 0, 1));
}

Chord::Chord(std::wstring s, glm::vec2 p){
	position=p;
	scale=glm::vec2(0, 0);
	shaders=0;
	vertexbuffer=0;
	const GLFWvidmode*mode=glfwGetVideoMode(glfwGetPrimaryMonitor());
	displaywidth=mode->width; displayheight=mode->height;
	vertices=std::vector<glm::vec2>(0);
	formatted=parse(s);
	text=Text(formatted, position, scale, 2, glm::vec4(0, 0, 0, 1));
}

void Chord::draw(){
	text.draw();
}

void Chord::setPosition(glm::vec2 p){
	position=p;
	text.setPosition(p);
}

std::wstring Chord::parse(std::wstring s){
	//very incomplete
	//how to handle 11s and 13s??
	//change to semi-tones instead of whole-tones?
	std::vector<float> n={0.0f, 3.5f};
	float accidental=0.0f;
	bool minor=false, extensions=false;
	std::wstring formatted=L"";
	root=L"";
	for(int x=0;x<s.length();x++){
		if(x==0){
			root+=s[x];
			formatted+=s[x];
			if(s.length()>x+1){
				if(s[x+1]=='#'||s[x+1]=='b'){
					root+=s[x+1];
					x++;
					if(s[x]=='#') formatted+=std::wstring(1, 0xE10C);
					else formatted+=std::wstring(1, 0xE10D);
				}
			}
		}
		else if(s[x]=='^'){
			extensions=true;
		}
		else if(s[x]=='m'||s[x]=='-'){
			//add parentheses functionality
			if(s.length()>x+3&&s.substr(x, 4)==L"maj7"){
				n.push_back(5.5f);
				x+=3;
				formatted+=L"maj7";
			}
			else{
				n.push_back(1.5f);
				minor=true;
				formatted+=s[x];
			}
		}
		if(extensions){
			accidental=0.0f;
			if(s[x]=='b'){
				if(s.length()>x+1){
					accidental=-0.5f;
					formatted+=std::wstring(1, 0xE188);
					x++;
				}
			}
			else if(s[x]=='#'){
				if(s.length()>x+1){
					accidental=0.5f;
					formatted+=std::wstring(1, 0xE189);
					x++;
				}
			}
			if(s[x]=='7'){
				n.push_back(5.0f);
				formatted+=std::wstring(1, 0xE197);
			}
			else if(s[x]=='6'){
				n.push_back(4.5f);
				formatted+=std::wstring(1, 0xE196);
			}
			else if(s[x]=='5'&&accidental!=0.0f){
				n.push_back(3.5f+accidental);
				formatted+=std::wstring(1, 0xE195);
			}
			else if(s[x]=='9'){
				n.push_back(7.0f+accidental);
				formatted+=std::wstring(1, 0xE199);
			}
			else if(s[x]=='1'){
				if(s.length()>x+1){
					x++;
					if(s[x]=='1'){
						n.push_back(8.5f+accidental);
						formatted+=std::wstring(1, 0xE191)+std::wstring(1, 0xE191);
					}
					else if(s[x]=='3'){
						n.push_back(10.5f+accidental);
						formatted+=std::wstring(1, 0xE191)+std::wstring(1, 0xE193);
					}
				}
			}
		}
	}
	if(!minor){n.push_back(2.0f);}
	notes=n;
	return formatted;
}

void Chord::setTextRaw(std::wstring t){
	formatted=parse(t);
	text.setText(formatted);
}