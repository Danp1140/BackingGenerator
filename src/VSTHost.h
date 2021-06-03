//
// Created by Daniel Paavola on 2020-08-17.
//

#ifndef BACKINGGENERATOR_VSTHOST_H
#define BACKINGGENERATOR_VSTHOST_H

#include <vst2/aeffect.h>
#include <vst2/aeffectx.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <map>
#include </Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/CoreFoundation.framework/Headers/CoreFoundation.h>
#include "Chord.h"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512

struct TimeData{
	unsigned int measure, beat, percent, minute, second, millisecond;
	bool operator<(const TimeData&t) const{
		//could probably gain some efficiency
		if(this->measure<t.measure) return true;
		else if(this->measure>t.measure) return false;
		if(this->measure==t.measure&&this->beat<t.beat) return true;
		if(this->measure==t.measure&&this->beat>t.beat) return false;
//		std::cout<<"this->percent: "<<this->percent<<" t.percent: "<<t.percent<<std::endl;
		if(this->measure==t.measure&&this->beat==t.beat&&this->percent<t.percent) return true;
		return false;
	}
	bool operator==(const TimeData&t) const{
		return this->measure==t.measure&&this->beat==t.beat&&this->percent==t.percent;
	}
	bool operator<=(const TimeData&t) const{
		bool less=false, equal=false;
		equal=this->measure==t.measure&&this->beat==t.beat&&this->percent==t.percent;
		if(this->measure<t.measure) less=true;
		else if(this->measure>t.measure) less=false;
		if(this->measure==t.measure&&this->beat<t.beat) less=true;
		if(this->measure==t.measure&&this->beat>t.beat) less=false;
		if(this->measure==t.measure&&this->beat==t.beat&&this->percent<t.percent) less=true;
		return less||equal;
	}
	TimeData operator+(const TimeData&t) const{
		//notably, this operator doesn't (can't) consider bounds of time signature/song
		TimeData result;
		result.measure=this->measure+t.measure;
		result.beat=this->beat+t.beat;
		result.percent=this->percent+t.percent;
		result.minute=this->minute+t.minute;
		result.second=this->second+t.second;
		result.millisecond=this->millisecond+t.millisecond;
		return result;
	}
	friend std::ostream& operator<<(std::ostream&o, const TimeData&t){
		o<<t.measure<<'.'<<t.beat<<'.'<<t.percent<<' '<<t.minute<<':'<<t.second<<'.'<<t.millisecond;
		return o;
	}
};

enum InstrumentType {
	DRUMS,
	KEYS
};

enum BassTonicMode {
	LOWEST,
	DOWN,
	NEAREST,
	UP,
	HIGHEST
};

//DjinnDrum/relative standard
//const std::map<int, int> drumsoundtokeymap={std::pair<int, int>(0, 0x33),
//                                            std::pair<int, int>(1, 0x2c),
//                                            std::pair<int, int>(2, 0x26),
//                                            std::pair<int, int>(3, 0x24)};
//Line of Legends
const std::map<int, int> drumsoundtokeymap={std::pair<int, int>(0, 0x4b),
                                            std::pair<int, int>(1, 0x48),
                                            std::pair<int, int>(2, 0x2d),
                                            std::pair<int, int>(3, 0x24)};

//scale degrees: 0 => 1, 3 => b3, 6 => b5, etc.
const int ionianscale[]={0, 2, 4, 5, 7, 9, 11};
const int dorianscale[]={0, 2, 3, 5, 7, 9, 10};
const int lydiandominantscale[]={0, 2, 4, 6, 7, 9, 10};
const int halfwholediminishedscale[]={0, 1, 3, 4, 6, 7, 9, 10};
const int mixolydianscale[]={0, 2, 4, 5, 7, 9, 10};

typedef AEffect *(*vstPluginFuncPtr)(audioMasterCallback);

typedef VstIntPtr (*dispatcherFuncPtr)(AEffect*effect, VstInt64 opCode, VstInt64 index, VstInt64 value, void*ptr, float opt);
typedef float (*getParameterFuncPtr)(AEffect*effect, VstInt64 index);
typedef void (*setParameterFuncPtr)(AEffect*effect, VstInt64 index, float value);
typedef VstInt32 (*processEventsFuncPtr)(VstEvents *events);
typedef void (*processFuncPtr)(AEffect*effect, float**inputs, float**outputs, VstInt64 sampleFrames);

class VSTHost{
private:
	AEffect*plugin;
	//putting VstEvents here was key in generating non-zero samples; why?
	VstEvents*e;
	std::vector<VstMidiEvent*> me;
	float**input, **output;
	//order is ride, hat, snare, bass
	//would like to be static, but architecture error happens
	float*parentdrumpattern[4], *currentdrumpattern[4];
	int drumpatternresolution;
	Chord currentchord, nextchord;
	int tonextchord;
	std::map<TimeData, char> basssequence;
public:

	VSTHost();

	VSTHost(const char*filepath, InstrumentType type);

	~VSTHost();

	static VstIntPtr VSTCALLBACK HostCallback(AEffect*effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void*ptr, float opt);

	static char rootToMidi(std::wstring root);

	void patternGenerationCallback(int populationcount, float mutationrate);

	void drumGeneticAlgorithm(int populationsize, float mutationrates[4]);

	void drumEventGenerationCallback(TimeData td, TimeData lasttd);

	void loadParentDrumPattern(const char*xmlfilepath, int tsnumerator);

	void bassGeneticAlgorithm(std::vector<std::vector<Chord>> c, TimeData td, TimeData lasttd, VstTimeInfo ti, BassTonicMode tm);

	std::map<TimeData, char> nextBassNote(Chord c, char startingnote, char currentnote, char targetnote, int beatstotarget, int totalbeats);

	void bassEventGenerationCallback(TimeData td, TimeData lasttd);

	void pianoVoicingAlgorithm(Chord c);

	void getSamples(float**i, float**o);

	void processAdding(float**i, float**o);

	int getNumInputs();

	int getNumOutputs();

	std::wstring getDisplayString();

	VstTimeInfo*getTimeInfo();
	//eventually make bool on into float velocity, negative velocity is off
	void addMidiEvent(char note, float velocity);

	void allMidiOff();

	void processE();
};


#endif //BACKINGGENERATOR_VSTHOST_H
