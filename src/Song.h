//
// Created by Daniel Paavola on 2020-06-01.
//

#ifndef BACKINGGENERATOR_SONG_H
#define BACKINGGENERATOR_SONG_H

#include <vector>
#include <map>
#include "VSTHost.h"
#include <vst2/aeffect.h>
#include <vst2/aeffectx.h>

#define SONG_VERTEX_SHADER_PATH "../resources/shaders/SongVertexShader.glsl"
#define SONG_FRAGMENT_SHADER_PATH "../resources/shaders/SongFragmentShader.glsl"

class Song:public Drawable{
private:
	int nummeasures, margins, verticalspacing, linethickness, measureheight, measuresperline;
	//consider using arrays instead of vectors
	std::vector<std::vector<Button>> slashes;
	std::vector<std::vector<Chord>> chords;
//	Chord**chords;
	std::map<Button*, Chord*> btocmap;
	VstTimeInfo ti;
	TimeData td;
	bool playing;
public:
	static GLuint songshaders;

	Song();

	~Song();

	void draw();

	void refreshVertices();

	void setPosition(glm::vec2 p);

	void setScale(glm::vec2 s);

	Button*getSlash(int m, int b);

	Chord*getChord(int m, int b);

	std::vector<std::vector<Button>> getSlashes(){return slashes;}

	std::vector<std::vector<Chord>> getChords(){return chords;}

	std::map<Button*, Chord*>*getBtoCMap(){return &btocmap;}

	int getTSNumerator(){return ti.timeSigNumerator;}
	//im really not thinking rn, double check if this is okay
	VstTimeInfo*getTimeInfo(){return &ti;}

	void advanceTimeData(unsigned int dmillisecond);

	TimeData getTimeData(){return td;}

	bool getPlaying(){return playing;}

	void togglePlaying();
};


#endif //BACKINGGENERATOR_SONG_H
