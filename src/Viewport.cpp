//
// Created by Daniel Paavola on 2020-04-30.
//

#include "Viewport.h"

GLFWwindow*Viewport::window=nullptr;
int Viewport::width=0, Viewport::height=0;

Viewport::Viewport(){
	glClearColor(1.0f, 1.0f, 0.9f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glGenVertexArrays(1, &vertexarray);
	glBindVertexArray(vertexarray);

	glfwSetCharCallback(window, charStreamCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	callbackdata->textinputbuffer=L"";
	glfwSetWindowUserPointer(window, callbackdata);
	activebutton=nullptr;

	times=new Text(L"", glm::vec2(4*width/5, height/5), glm::vec2(50, 50), 2, glm::vec4(0, 0, 0, 1));
	song=new Song();
	song->setPosition(glm::vec2(200, 0));
	song->setScale(glm::vec2(width-song->getPosition().x, height));
	ah=new AudioHandler(song);
	masterpeakmeterleft=Button(L"", glm::vec2(250, 100), glm::vec4(0, 1, 0, 1), 1.0f, BUTTON_TEXT_EDITOR);
	masterpeakmeterleft.setScale(glm::vec2(25, 0));
	masterpeakmeterright=Button(L"", glm::vec2(280, 100), glm::vec4(0, 1, 0, 1), 1.0f, BUTTON_TEXT_EDITOR);
	masterpeakmeterright.setScale(glm::vec2(25, 0));
	vstparams=Text(ah->getTest().bass->getDisplayString(), glm::vec2(50, 800), glm::vec2(1, 1), 2.0f, glm::vec4(0, 0, 0, 1));
}

Viewport::~Viewport(){
	delete times;
	delete ah;
	delete song;
	glDeleteVertexArrays(1, &vertexarray);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Viewport::initGL(){
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	const GLFWvidmode*mode=glfwGetVideoMode(glfwGetPrimaryMonitor());
	width=mode->width; height=mode->height;
	window=glfwCreateWindow(width, height, "window", glfwGetPrimaryMonitor(), nullptr);
	glfwGetWindowSize(window, &width, &height);
	glfwMakeContextCurrent(window);
	glewExperimental=GL_TRUE;
	glewInit();
	std::cout<<"OpenGL version: "<<glGetString(GL_VERSION)<<
	         "\nGLSL version: "<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<
	         "\nGLFW version: "<<glfwGetVersionString()<<std::endl;

}

void Viewport::draw(){
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	if(callbackdata->mousepress&&!callbackdata->lastmousepress){
		double xpos=0.0, ypos=0.0;
		glfwGetCursorPos(window, &xpos, &ypos);
		ypos=height-ypos;
		if((xpos>song->getPosition().x&&xpos<song->getPosition().x+song->getScale().x)
			&&(ypos>song->getPosition().y&&ypos<song->getPosition().y+song->getScale().y)){
			//further efficiency possible regarding measures
			Button*temp=nullptr;
			//ypos gets screwy when not fullscreened
			//b/c height != 900
			//vidmode height is not actually windowed height
			for(int m=0;m<song->getSlashes().size();m++){
				for(int b=0;b<song->getTSNumerator();b++){
					//note: only checks slashes
					temp=song->getSlash(m, b);
					if((xpos>temp->getPosition().x&&xpos<temp->getPosition().x+temp->getScale().x)
						&&ypos>temp->getPosition().y&&ypos<temp->getPosition().y+temp->getScale().y){
						activebutton=song->getSlash(m, b);
						//this should really be set elsewhere
						song->getSlash(m, b)->callback=buttonCallback;
						callbackdata->textinputbuffer=song->getChord(m, b)->getText()->getText();
						b=song->getTSNumerator(); m=song->getSlashes().size();

					}
				}
			}
		}
	}
	if(activebutton!=nullptr){
		int input[callbackdata->textinputbuffer.length()+1];
		for(int x=0;x<callbackdata->textinputbuffer.length();x++) input[x]=callbackdata->textinputbuffer[x];
		input[callbackdata->textinputbuffer.length()]='\0';
		if(activebutton->getType()==BUTTON_TEXT_EDITOR){
			if(!activebutton->callback(BUTTON_TEXT_EDITOR, activebutton, input)){
				activebutton=nullptr;
				callbackdata->textinputbuffer=L"";
			}
		}
		else if(activebutton->getType()==BUTTON_SLASH){
			if(!activebutton->callback(BUTTON_SLASH, song->getBtoCMap()->find(activebutton)->second, input)){
				song->getBtoCMap()->find(activebutton)->second->setTextRaw(callbackdata->textinputbuffer);
				activebutton=nullptr;
				callbackdata->textinputbuffer=L"";
			}
		}
	}
	else if(callbackdata->textinputbuffer[0]==L' '){
		song->togglePlaying();
		callbackdata->textinputbuffer=L"";
	}
	else callbackdata->textinputbuffer=L"";

	std::wstring secondcomp=L"", millisecondcomp=L"", percentcomp=L"";
	if(song->getTimeData().second<10){secondcomp=L"0";}
	if(song->getTimeData().millisecond<100){
		millisecondcomp=L"00";
		if(song->getTimeData().millisecond>10) millisecondcomp=L"0";
	}
	if(song->getTimeData().percent<10){percentcomp=L"0";}
	times->setText(std::to_wstring(song->getTimeData().measure)+L'.'
	+std::to_wstring(song->getTimeData().beat)+L'.'
	+percentcomp+std::to_wstring(song->getTimeData().percent)+L'\n'
	+std::to_wstring(song->getTimeData().minute)+L':'
	+secondcomp+std::to_wstring(song->getTimeData().second)+L'.'
	+millisecondcomp+std::to_wstring(song->getTimeData().millisecond));

	float currentsample=(song->getPlaying())?ah->getTest().output[0][int(song->getTimeInfo()->samplePos)%FRAMES_PER_BUFFER]:0;
	currentsample=abs(currentsample*1000);
	if(masterpeakmeterleft.getScale().y<currentsample) masterpeakmeterleft.setScale(masterpeakmeterleft.getScale()+glm::vec2(0, 10));
	else if(masterpeakmeterleft.getScale().y>currentsample) masterpeakmeterleft.setScale(masterpeakmeterleft.getScale()+glm::vec2(0, -10));
	float currentsampleright=(song->getPlaying())?ah->getTest().output[1][int(song->getTimeInfo()->samplePos)%FRAMES_PER_BUFFER]:0;
	currentsampleright=abs(currentsampleright*1000);
	if(masterpeakmeterright.getScale().y<currentsampleright) masterpeakmeterright.setScale(masterpeakmeterright.getScale()+glm::vec2(0, 10));
	else if(masterpeakmeterright.getScale().y>currentsampleright) masterpeakmeterright.setScale(masterpeakmeterright.getScale()+glm::vec2(0, -10));

	song->draw();

	times->draw();

	masterpeakmeterleft.draw();
	masterpeakmeterright.draw();

	vstparams.draw();

//	std::cout<<"window address: "<<window<<std::endl;
//okay random guess but try moving these to main instead of viewport
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Viewport::charStreamCallback(GLFWwindow*window, unsigned int codepoint){
	((inputdata*)glfwGetWindowUserPointer(window))->textinputbuffer.push_back(codepoint);
}

void Viewport::keyboardCallback(GLFWwindow*window, int key, int scancode, int action, int mods){
	inputdata*data=(inputdata*)glfwGetWindowUserPointer(window);
	if(key==GLFW_KEY_BACKSPACE&&action==GLFW_PRESS&&data->textinputbuffer.size()>0) data->textinputbuffer.erase(data->textinputbuffer.end()-1);
	else if(key==GLFW_KEY_ENTER&&action==GLFW_PRESS) data->textinputbuffer.push_back('\n');
}

void Viewport::mouseButtonCallback(GLFWwindow*window, int button, int action, int mods){
	inputdata*data=(inputdata*)glfwGetWindowUserPointer(window);
	data->lastmousepress=data->mousepress;
	if(button==GLFW_MOUSE_BUTTON_1&&action==GLFW_PRESS) data->mousepress=true;
//	else if(button==GLFW_MOUSE_BUTTON_1&&action==GLFW_RELEASE) data->mousepress=false;
	else data->mousepress=false;
}

bool Viewport::buttonCallback(ButtonType bt, ...){
	if(bt==BUTTON_TEXT_EDITOR){
		va_list l;
		va_start(l, 2);
		Button*edit=va_arg(l, Button*);
		int*input=va_arg(l, int*);
		std::wstring temp=L"";
		for(int x=0;;x++){
			if(input[x]=='\0') break;
			else if(input[x]=='\n') return false;
			temp+=input[x];
		}
		edit->message.setText(temp);
		va_end(l);
		return true;
	}
	if(bt==BUTTON_SLASH){
		va_list l;
		va_start(l, 2);
		Chord*edit=va_arg(l, Chord*);
		int*input=va_arg(l, int*);
		std::wstring temp=L"";
		for(int x=0;;x++){
			if(input[x]=='\0') break;
			else if(input[x]=='\n') return false;
			temp+=input[x];
		}
		edit->getText()->setText(temp);
		va_end(l);
		return true;
	}
	return false;
}