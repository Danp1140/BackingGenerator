//
// Created by Daniel Paavola on 2020-06-01.
//

#include "Song.h"

GLuint Song::songshaders=-1;

Song::Song(){
	position=glm::vec2(0, 0);
	nummeasures=4;
	margins=100;
	verticalspacing=100;
	linethickness=2;
	measureheight=40;
	measuresperline=4;
	td={1, 1, 0, 0, 0, 0};
	playing=false;

	ti.tempo=300.0f;
	ti.timeSigNumerator=4;
	ti.timeSigDenominator=4;
	ti.sampleRate=SAMPLE_RATE;
	ti.samplePos=0;
	ti.ppqPos=0;
	ti.flags=kVstPpqPosValid|kVstTempoValid|kVstTimeSigValid;

	const GLFWvidmode*mode=glfwGetVideoMode(glfwGetPrimaryMonitor());
	displaywidth=mode->width; displayheight=mode->height;
	scale=glm::vec2(displaywidth, displayheight);
	if(songshaders==-1) songshaders=Drawable::loadShaders(SONG_VERTEX_SHADER_PATH, SONG_FRAGMENT_SHADER_PATH);
	shaders=songshaders;
	slashes=std::vector<std::vector<Button>>();
	chords=std::vector<std::vector<Chord>>();
	btocmap=std::map<Button*, Chord*>();
	for(int x=0;x<nummeasures;x++){
		chords.push_back(std::vector<Chord>());
		slashes.push_back(std::vector<Button>());
		for(int y=0;y<ti.timeSigNumerator;y++){
			chords[x].push_back(Chord(L"", glm::vec2(0, 0)));
			slashes[x].push_back(Button(L"/", glm::vec2(0, 0), glm::vec4(0, 0, 0, 0), 2, BUTTON_SLASH));
		}
	}
	for(int x=0;x<nummeasures;x++){
		for(int y=0;y<ti.timeSigNumerator;y++){
			btocmap.insert(std::pair<Button*, Chord*>(&slashes[x][y], &chords[x][y]));
		}
	}
	glGenBuffers(1, &vertexbuffer);
	refreshVertices();
}

Song::~Song(){
	glDeleteBuffers(1, &vertexbuffer);
}

void Song::draw(){
	for(int x=0;x<slashes.size();x++){
		for(int y=0;y<slashes[x].size();y++){
			chords[x][y].draw();
			slashes[x][y].draw();
		}
	}

	glUseProgram(shaders);
	GLint widthloc=glGetUniformLocation(shaders, "width");
	GLint heightloc=glGetUniformLocation(shaders, "height");
	glUniform1i(widthloc, displaywidth);
	glUniform1i(heightloc, displayheight);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
	glDisableVertexAttribArray(0);
}

void Song::refreshVertices(){
	vertices=std::vector<glm::vec2>(0);
	int line=0;
	float width=(scale.x-(2*margins))/measuresperline;
	for(int x=0;x<slashes.size();x++){
		vertices.emplace_back(margins+(x*width)-(line*measuresperline*width)-(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight));
		vertices.emplace_back(margins+(x*width)-(line*measuresperline*width)-(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-measureheight);
		vertices.emplace_back(margins+(x*width)-(line*measuresperline*width)+(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-measureheight);

		vertices.emplace_back(margins+(x*width)-(line*measuresperline*width)+(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-measureheight);
		vertices.emplace_back(margins+(x*width)-(line*measuresperline*width)+(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight));
		vertices.emplace_back(margins+(x*width)-(line*measuresperline*width)-(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight));

		for(int y=0;y<5;y++){
			vertices.emplace_back(margins+(x*width)-(line*measuresperline*width)+(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-float(y)*(measureheight-linethickness)/4);
			vertices.emplace_back(margins+(x*width)-(line*measuresperline*width)+(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-float(y)*(measureheight-linethickness)/4-linethickness);
			vertices.emplace_back(margins+((x+1)*width)-(line*measuresperline*width)-(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-float(y)*(measureheight-linethickness)/4-linethickness);

			vertices.emplace_back(margins+((x+1)*width)-(line*measuresperline*width)-(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-float(y)*(measureheight-linethickness)/4-linethickness);
			vertices.emplace_back(margins+((x+1)*width)-(line*measuresperline*width)-(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-float(y)*(measureheight-linethickness)/4);
			vertices.emplace_back(margins+(x*width)-(line*measuresperline*width)+(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-float(y)*(measureheight-linethickness)/4);

		}

		for(int y=0;y<ti.timeSigNumerator;y++){
			slashes[x][y].setPosition(position+glm::vec2(margins+20+(x*width+(y*width/ti.timeSigNumerator))-measuresperline*width*line, scale.y-margins-line*(verticalspacing+measureheight)-0.7*measureheight));
			chords[x][y].setPosition(slashes[x][y].getPosition()+glm::vec2(0, measureheight));
		}

		if(x%measuresperline==measuresperline-1||x==slashes.size()-1){
			vertices.emplace_back(margins+((x+1)*width)-(line*measuresperline*width)-(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight));
			vertices.emplace_back(margins+((x+1)*width)-(line*measuresperline*width)-(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-measureheight);
			vertices.emplace_back(margins+((x+1)*width)-(line*measuresperline*width)+(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-measureheight);

			vertices.emplace_back(margins+((x+1)*width)-(line*measuresperline*width)+(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight)-measureheight);
			vertices.emplace_back(margins+((x+1)*width)-(line*measuresperline*width)+(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight));
			vertices.emplace_back(margins+((x+1)*width)-(line*measuresperline*width)-(float(linethickness)/2.0f), scale.y-margins-line*(verticalspacing+measureheight));
			line++;
		}
	}

	for(auto&v:vertices) v+=position;

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, -1); //neccesary???
}

void Song::setPosition(glm::vec2 p){
	position=p;
	refreshVertices();
}

void Song::setScale(glm::vec2 s){
	scale=s;
	refreshVertices();
}

Button*Song::getSlash(int m, int b){
	return &slashes[m][b];
}

Chord*Song::getChord(int m, int b){
	return &chords[m][b];
}

void Song::advanceTimeData(unsigned int dmillisecond){
	if(playing){
		td.millisecond+=dmillisecond;
		if(td.millisecond>999){
			td.millisecond=0;
			td.second++;
			if(td.second>59){
				td.second=0;
				td.minute++;
			}
		}
		unsigned int totalmilli=td.millisecond+1000*td.second+60000*td.minute;
		td.percent=(unsigned int)(float(totalmilli)/1000.0f*ti.tempo/60.0f*100)%100;
		td.beat=((unsigned int)(float(totalmilli)/1000.0f*ti.tempo/60.0f*100)/100)%ti.timeSigNumerator+1;
		td.measure=((((unsigned int)((totalmilli/1000.0*(ti.tempo/60.0))*100.0))/100)/ti.timeSigNumerator)%slashes.size()+1;
		ti.ppqPos=1.0f/double(ti.timeSigDenominator)*4.0f*(0.01f*double(td.percent)+double(td.beat)+double(td.measure*ti.timeSigNumerator));
	}
}


void Song::togglePlaying(){
	playing=!playing;
}