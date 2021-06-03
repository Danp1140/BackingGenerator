//
// Created by Daniel Paavola on 2020-04-30.
//

#include "Text.h"

GLuint Text::textshaders=-1;
GLuint*Text::chartextures=nullptr;
std::map<wchar_t, GLuint> Text::outofrangechartextures=std::map<wchar_t, GLuint>();

Text::Text():text(std::wstring()), fontsize(1), fontcolor(glm::vec4(1, 1, 1, 1)){
	position=glm::vec2(0, 0);
	scale=glm::vec2(0, 0);
	displaywidth=0; displayheight=0;
//	const GLFWvidmode*mode=glfwGetVideoMode(glfwGetPrimaryMonitor());
//	displaywidth=mode->width; displayheight=mode->height;
	FT_Library library=FT_Library();
	if(FT_Init_FreeType(&library)!=0){std::cout<<"Freetype initialization error"<<std::endl;}
	FT_New_Face(library, "../resources/fonts/arial.ttf", 0, &face);
	FT_Set_Char_Size(face, fontsize*0.6*64, fontsize*64, displaywidth, displayheight); //is 64 necessary?
//	if(textshaders==-1) textshaders=loadShaders(TEXT_VERTEX_SHADER_PATH, TEXT_FRAGMENT_SHADER_PATH);
//	shaders=textshaders;
////	textures=std::vector<GLuint>();
//	refreshVertices();
//	if(chartextures==nullptr){
//		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//		loadCharTextures();
//	}
//	glGenBuffers(1, &vertexbuffer);
//	glGenBuffers(1, &uvbuffer);
//	std::cout<<"text obj at "<<this<<" with vbuff "<<vertexbuffer<<" and uvbuff "<<uvbuffer<<" null initialized"<<std::endl;
}

Text::Text(std::wstring t, glm::vec2 p, glm::vec2 s, int fs, glm::vec4 c):text(t), fontsize(fs), fontcolor(c){
	position=p;
	scale=s;
	const GLFWvidmode*mode=glfwGetVideoMode(glfwGetPrimaryMonitor());
	displaywidth=mode->width; displayheight=mode->height;
	lib=FT_Library();
	if(FT_Init_FreeType(&lib)!=0){std::cout<<"Freetype initialization error"<<std::endl;}
	FT_New_Face(lib, "../resources/fonts/MuseJazz.ttf", 0, &face);
	FT_Set_Char_Size(face, fontsize*0.6*TEXT_SCALING_FACTOR, fontsize*TEXT_SCALING_FACTOR, displaywidth, displayheight); //is 64 necessary?
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//	glGenBuffers(1, &vertexbuffer);
//	glGenBuffers(1, &uvbuffer);
//	std::cout<<"text obj at "<<this<<" with vbuff "<<vertexbuffer<<" and uvbuff "<<uvbuffer<<" initialized"
//	<<std::endl;
	if(textshaders==-1) textshaders=loadShaders(TEXT_VERTEX_SHADER_PATH, TEXT_FRAGMENT_SHADER_PATH);
	shaders=textshaders;
	refreshVertices();
	if(chartextures==nullptr) loadCharTextures();
}

Text::~Text(){
//	std::cout<<"text obj at "<<this<<" with vbuff "<<vertexbuffer<<" and uvbuff "<<uvbuffer<<" destroyed"<<std::endl;
//	glDeleteBuffers(1, &vertexbuffer);
//	glDeleteBuffers(1, &uvbuffer);
}

void Text::refreshVertices(){
	//REMEMBER: this is not called once or twice, it is called EVERY LOOP when song is playing
	vertices=std::vector<glm::vec2>(0);
	textures=std::vector<GLuint>(text.length());
	float linepos=0, rowpos=0;
	FT_UInt index;
	FT_Glyph_Metrics m;
	for(int x=0;x<text.length();x++){
		wchar_t c=text[x];
		if(c=='\n'){
			linepos=0;
			rowpos+=float(face->glyph->metrics.height)/float(TEXT_SCALING_FACTOR)+10.0f;
			continue;
		}
		index=FT_Get_Char_Index(face, c);
		FT_Load_Glyph(face, index, FT_LOAD_RENDER);
		if(face->glyph!=nullptr) FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		m=face->glyph->metrics;
		scale=glm::vec2(m.width/TEXT_SCALING_FACTOR, m.height/TEXT_SCALING_FACTOR);
		glm::vec2 bearing=glm::vec2(m.horiBearingX/TEXT_SCALING_FACTOR, m.horiBearingY/TEXT_SCALING_FACTOR);
		vertices.push_back(position+glm::vec2(linepos, -rowpos)+bearing+glm::vec2(scale.x, 0.0f));
		vertices.push_back(position+glm::vec2(linepos, -rowpos)+bearing+glm::vec2(0.0f, 0.0f));
		vertices.push_back(position+glm::vec2(linepos, -rowpos)+bearing+glm::vec2(0.0f, -scale.y));
		vertices.push_back(position+glm::vec2(linepos, -rowpos)+bearing+glm::vec2(0.0f, -scale.y));
		vertices.push_back(position+glm::vec2(linepos, -rowpos)+bearing+glm::vec2(scale.x, -scale.y));
		vertices.push_back(position+glm::vec2(linepos, -rowpos)+bearing+glm::vec2(scale.x, 0.0f));
		linepos+=m.horiAdvance/TEXT_SCALING_FACTOR;
	}
}

void Text::loadCharTextures(){
	//doesn't account for multiple fonts
	chartextures=new GLuint[0x7e];
	for(int x=0x21;x<=0x7e;x++){
		FT_Load_Glyph(face, FT_Get_Char_Index(face, wchar_t(x)), FT_LOAD_RENDER);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		glGenTextures(1, &chartextures[x]);
		glBindTexture(GL_TEXTURE_2D, chartextures[x]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED,
		             GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	for(auto&c:outofrangecharstoload){
		outofrangechartextures.emplace(c, int(c));
	}
	for(auto&c:outofrangecharstoload){
		FT_Load_Glyph(face, FT_Get_Char_Index(face, c), FT_LOAD_RENDER);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		GLuint*texid=&(outofrangechartextures.find(c)->second);
		glGenTextures(1, texid);
		glBindTexture(GL_TEXTURE_2D, *texid);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED,
		             GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
}

void Text::draw(){
//	std::cout<<"text object "<<this<<" being drawn"<<std::endl;
	glUseProgram(shaders);
	GLint widthloc=glGetUniformLocation(shaders, "width");
	GLint heightloc=glGetUniformLocation(shaders, "height");
	GLint fontcolorloc=glGetUniformLocation(shaders, "fontcolor");
	glUniform1i(widthloc, displaywidth);
	glUniform1i(heightloc, displayheight);
	glUniform4fv(fontcolorloc, 1, &fontcolor[0]);

	glGenBuffers(1, &vertexbuffer);
	glGenBuffers(1, &uvbuffer);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, 6*sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	int counter=0;
	for(int x=0;x<text.length();x++){
		if(text[x]!=L'\n'){
//			std::cout<<"\tcharacter generation ("<<x<<" in string)"<<std::endl;
			if(int(text[x])<0x7e) glBindTexture(GL_TEXTURE_2D, chartextures[text[x]]);
			//should add a contingency if below is not found
			else glBindTexture(GL_TEXTURE_2D, outofrangechartextures.at(text[x]));

			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, 6*sizeof(glm::vec2), &vertices[6*counter], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

			counter++;
		}
	}
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
}

void Text::setPosition(glm::vec2 p){
	position=p;
	refreshVertices();
}

void Text::setText(std::wstring t){
	text=t;
	if(t.length()>MAX_TEXT_LENGTH) std::cout<<"warning: max text length exceeded. some characters will not display"<<std::endl;
	refreshVertices();
}

void Text::setFontSize(int fs){
	fontsize=fs;
	FT_Set_Char_Size(face, fontsize*0.6*64, fontsize*64, displaywidth, displayheight); //is 64 necessary?
	refreshVertices();
}