//
// Created by Daniel Paavola on 2020-07-02.
//

#ifndef BACKINGGENERATOR_AUDIOHANDLER_H
#define BACKINGGENERATOR_AUDIOHANDLER_H

#include <PortAudio/portaudio.h>
#include <vector>
#include <queue>
#include "VSTHost.h"
#include "Song.h"

typedef struct{
	//make vsts non-pointers
	//for some reason, vsts have to be pointers to avoid a crash on a destructor (maybe because of null initializer)
	//make destructor
	VSTHost*vst, *drums, *bass;
	Song*s;
	std::chrono::time_point<std::chrono::high_resolution_clock> lasttime;
	float**input, **output;
} AudioCallbackData;

class AudioHandler{
private:
	PaStream*stream;
	static int playbackCallback(const void*inputBuffer, void*outputBuffer, unsigned long framesPerBuffer,
			const PaStreamCallbackTimeInfo*timeInfo, PaStreamCallbackFlags statusFlags, void*userData);

	AudioCallbackData callbackdata;
public:
	//Generates default initialized objects or nullptrs for all essential AudioHandler members
	AudioHandler();

	AudioHandler(Song*s);

	AudioCallbackData getTest(){return callbackdata;}

	~AudioHandler();
};


#endif //BACKINGGENERATOR_AUDIOHANDLER_H
