//
// Created by Daniel Paavola on 2020-07-02.
//

#include <iostream>
#include "AudioHandler.h"

AudioHandler::AudioHandler(){
	callbackdata={nullptr, nullptr, nullptr, nullptr, std::chrono::high_resolution_clock::now(), nullptr, nullptr};
	stream=nullptr;
	std::cout<<"An AudioHandler has been default initalized ("<<this<<')'<<std::endl;
}

AudioHandler::AudioHandler(Song*s) {
	Pa_Initialize();
	PaStreamParameters outputParameters;
	for(int x=0;x<Pa_GetDeviceCount();x++) std::cout<<x<<": "<<Pa_GetDeviceInfo(x)->name<<", "<<Pa_GetDeviceInfo(x)->maxInputChannels<<" in, "<<Pa_GetDeviceInfo(x)->maxOutputChannels<<" out"<<std::endl;
	PaDeviceIndex index;
	std::cin>>index;
	outputParameters.hostApiSpecificStreamInfo=nullptr;
	outputParameters.device=index;
	outputParameters.sampleFormat=paFloat32;
	outputParameters.channelCount=2;
	outputParameters.suggestedLatency=Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->defaultLowOutputLatency;

	callbackdata={new VSTHost("../resources/vsts/Keyzone Classic.vst", KEYS), new VSTHost("../resources/vsts/Line of Legends.vst", DRUMS), new VSTHost("../resources/vsts/ABPL2.vst", KEYS), s, std::chrono::high_resolution_clock::now(), nullptr, nullptr};
	callbackdata.input=new float*[callbackdata.vst->getNumInputs()];
	for(int x=0;x<callbackdata.vst->getNumInputs();x++){
		callbackdata.input[x]=new float[FRAMES_PER_BUFFER];
		memset(callbackdata.input[x], 0, FRAMES_PER_BUFFER*sizeof(float));
	}
	callbackdata.output=new float*[callbackdata.vst->getNumOutputs()];
	for(int x=0;x<callbackdata.vst->getNumOutputs();x++){
		callbackdata.output[x]=new float[FRAMES_PER_BUFFER];
		memset(callbackdata.output[x], 0, FRAMES_PER_BUFFER*sizeof(float));
	}

	stream=nullptr;
	Pa_OpenStream(&stream, nullptr, &outputParameters, double(SAMPLE_RATE), (unsigned long)(FRAMES_PER_BUFFER), paClipOff, playbackCallback, &callbackdata);
	Pa_StartStream(stream);

	std::cout<<"An AudioHandler has been parametrically initialized ("<<this<<')'<<std::endl;
}

AudioHandler::~AudioHandler(){
	Pa_StopStream(stream);
	Pa_CloseStream(stream);
	Pa_Terminate();
	std::cout<<"An AudioHandler has been destroyed ("<<this<<')'<<std::endl;
}

int AudioHandler::playbackCallback(const void*inputBuffer, void*outputBuffer, unsigned long framesPerBuffer,
                                   const PaStreamCallbackTimeInfo*timeInfo, PaStreamCallbackFlags statusFlags, void*userData){
	float*write=(float*)outputBuffer;
	AudioCallbackData*data=(AudioCallbackData*)userData;

	bool timezero=data->s->getTimeData().millisecond+data->s->getTimeData().second+data->s->getTimeData().minute==0;
	TimeData temptime=data->s->getTimeData();
	data->s->advanceTimeData(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-data->lasttime).count());
	data->lasttime=std::chrono::high_resolution_clock::now();
	if(data->s->getPlaying()){
		if(temptime.beat!=data->s->getTimeData().beat||timezero){
			data->bass->bassGeneticAlgorithm(data->s->getChords(), data->s->getTimeData(), temptime, *data->s->getTimeInfo(), HIGHEST);

			Chord*temp=data->s->getChord(data->s->getTimeData().measure-1, data->s->getTimeData().beat-1);
			if(temp!=nullptr){
				std::wstring r=temp->getRoot();
				char n;
				if(r.length()>0){
					if(r[0]==L'A') n=0x39;
					else if(r[0]==L'B') n=0x3b;
					else if(r[0]==L'C') n=0x3c;
					else if(r[0]==L'D') n=0x3e;
					else if(r[0]==L'E') n=0x40;
					else if(r[0]==L'F') n=0x41;
					else if(r[0]==L'G') n=0x43;
					if(r.length()>1){
						if(r[1]==L'#') n++;
						else if(r[1]=='b') n--;
					}
//					data->bass->allMidiOff();
//					data->bass->addMidiEvent(n-1r2, 1.0f);
//					data->bass->processE();
					data->vst->allMidiOff();
					for(int x=1;x<temp->getNotes().size();x++){
						data->vst->addMidiEvent((n+temp->getNotes()[x]*2)+12, 1.0f);
					}
					data->vst->processE();
				}
			}
		}
		data->vst->getSamples(data->input, data->output);
		data->bass->bassEventGenerationCallback(data->s->getTimeData(), temptime);
		data->bass->processAdding(data->input, data->output);

		if(temptime.measure!=data->s->getTimeData().measure){
			float mutations[4]={0.0f, 0.0f, 0.00f, 0.00f};
			data->drums->drumGeneticAlgorithm(30, mutations);
		}
		data->drums->drumEventGenerationCallback(data->s->getTimeData(), temptime);
//		data->drums->processAdding(data->input, data->output);

		for(int x=0;x<framesPerBuffer;x++){
			for(int y=0;y<2;y++) *write++=data->output[y][x];
			(data->s->getTimeInfo()->samplePos)++;
			if(data->s->getTimeInfo()->samplePos==SAMPLE_RATE) data->s->getTimeInfo()->samplePos=0;
		}
	}
	else{
		for(int x=0;x<framesPerBuffer;x++){
			for(int y=0;y<2;y++) *write++=0.0f;
		}
	}
	return paContinue;
}