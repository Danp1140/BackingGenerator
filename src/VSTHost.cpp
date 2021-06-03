//
// Created by Daniel Paavola on 2020-08-17.
//

#include "VSTHost.h"

VSTHost::VSTHost(){
	plugin=nullptr;
	e=new VstEvents();
	input=nullptr; output=nullptr;
	for(int x=0;x<4;x++){
		parentdrumpattern[x]=nullptr;
		currentdrumpattern[x]=nullptr;
	}
	drumpatternresolution=-1;
	currentchord=Chord(L"", glm::vec2(0, 0)); nextchord=Chord(L"", glm::vec2(0, 0));
	tonextchord=0;
	std::cout<<"vsthost default initialized ("<<this<<')'<<std::endl;
}

VSTHost::VSTHost(const char*filepath, InstrumentType type){
	CFStringRef vstpathsr=CFStringCreateWithCString(nullptr, filepath, kCFStringEncodingASCII);
	CFURLRef bundleurl=CFURLCreateWithFileSystemPath(kCFAllocatorDefault, vstpathsr, kCFURLPOSIXPathStyle, true);
	CFBundleRef bundleref=CFBundleCreate(kCFAllocatorDefault, bundleurl);
	vstPluginFuncPtr mainentrypoint=(vstPluginFuncPtr)CFBundleGetFunctionPointerForName(bundleref, CFSTR("VSTPluginMain"));
	if(mainentrypoint==nullptr) mainentrypoint=(vstPluginFuncPtr)CFBundleGetFunctionPointerForName(bundleref, CFSTR("main_macho"));
	if(mainentrypoint==nullptr) mainentrypoint=(vstPluginFuncPtr)CFBundleGetFunctionPointerForName(bundleref, CFSTR("main"));
	plugin=mainentrypoint(HostCallback);
	dispatcherFuncPtr dispatcher=(dispatcherFuncPtr)(plugin->dispatcher);
	plugin->dispatcher(plugin, effOpen, 0, 0, 0, 0);
	plugin->dispatcher(plugin, effSetSampleRate, 0, 0, 0, SAMPLE_RATE);
	plugin->dispatcher(plugin, effSetBlockSize, 0, FRAMES_PER_BUFFER, 0, 0);

	bool printstuff=true;
	if(printstuff) std::wcout<<this->getDisplayString()<<std::endl;
//	if(printstuff){
//		char name[256], vendor[256], product[256];
//		plugin->dispatcher(plugin, effGetEffectName, 0, 0, name, 0);
//		plugin->dispatcher(plugin, effGetVendorString, 0, 0, vendor, 0);
//		plugin->dispatcher(plugin, effGetProductString, 0, 0, product, 0);
//		std::cout<<"Plugin name: "<<name<<"\nVendor: "<<vendor<<"\nProduct: "<<product<<std::endl;
//		std::cout<<plugin->numInputs<<" input channels | "<<plugin->numOutputs<<" output channels"<<std::endl;
//		std::cout<<"Parameters: "<<std::endl;
//		char paramname[256], paramdisplay[256], paramlabel[256];
//		for(int x=0;x<plugin->numParams;x++){
//			plugin->dispatcher(plugin, effGetParamName, x, 0, paramname, 0);
//			plugin->dispatcher(plugin, effGetParamDisplay, x, 0, paramdisplay, 0);
//			plugin->dispatcher(plugin, effGetParamLabel, x, 0, paramlabel, 0);
//			std::cout<<"\t"<<paramname<<": "<<paramdisplay<<paramlabel<<std::endl;
//		}
//		char programname[256];
//		std::cout<<"Programs: "<<std::endl;
//		for(int x=0;x<plugin->numPrograms;x++){
//			plugin->dispatcher(plugin, effSetProgram, 0, x, 0, 0);
//			plugin->dispatcher(plugin, effGetProgramName, 0, 0, programname, 0);
//			std::cout<<"\t"<<programname<<std::endl;
//		}
//		const char*candos[]={"canDoReceiveVstEvents", "canDoReceiveMidiVstEvent", "canDoReceiveVstTimeInfo",
//					   "canDoSendVstEvents", "canDoSendMidiVstEvent"};
//		for(int x=0;x<sizeof(candos)/sizeof(candos[0]);x++){
//			std::cout<<candos[x]<<": "<<plugin->dispatcher(plugin, effCanDo, 0, 0, (void*)candos[x], 0)<<std::endl;
//		}
//	}

	e=new VstEvents();

	if(type==DRUMS) loadParentDrumPattern("../resources/templates/swingdrums.xml", 4);
	currentchord=Chord(L"", glm::vec2(0, 0));
	nextchord=Chord(L"", glm::vec2(0, 0));
	basssequence=std::map<TimeData, char>();

	input=new float*[plugin->numInputs];
	for(int x=0;x<plugin->numInputs;x++){
		input[x]=new float[FRAMES_PER_BUFFER];
		memset(input[x], 0, FRAMES_PER_BUFFER*sizeof(float));
	}
	output=new float*[plugin->numOutputs];
	for(int x=0;x<plugin->numOutputs;x++){
		output[x]=new float[FRAMES_PER_BUFFER];
		memset(output[x], 0, FRAMES_PER_BUFFER*sizeof(float));
	}

	plugin->dispatcher(plugin, effMainsChanged, 0, 1, 0, 0.0f);
	std::cout<<"vsthost parametrically initialized ("<<this<<')'<<std::endl;
}

VSTHost::~VSTHost(){
	for(int x=0;x<4;x++){
		if(parentdrumpattern!=nullptr&&parentdrumpattern[x]!=nullptr) delete parentdrumpattern[x];
		if(currentdrumpattern!=nullptr&&currentdrumpattern[x]!=nullptr) delete currentdrumpattern[x];
	}
}

VstIntPtr VSTCALLBACK VSTHost::HostCallback(AEffect*effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void*ptr, float opt){
	switch(opcode){
		case audioMasterAutomate:
			//idk what to return, but ModulAir really wants this one
			return 0;
		case audioMasterVersion:
			return 2400;
		case audioMasterGetSampleRate:
			return SAMPLE_RATE;
		case audioMasterGetBlockSize:
			return FRAMES_PER_BUFFER;
		case __audioMasterWantMidiDeprecated:
			return 1; //unsure of what to return
		case audioMasterGetTime:
			return NULL;
		case audioMasterIOChanged:
			//idk man
			return 0;
		case audioMasterGetCurrentProcessLevel:
			//figure this out properly, returns should be decipherable
			return kVstProcessLevelUnknown;
		case audioMasterUpdateDisplay:
			//this is super fishy
			return 0;
		default:
			std::cout<<"unimplemented opcode requested: "<<opcode<<std::endl;
			return 0;
	}
}

char VSTHost::rootToMidi(std::wstring root){
	int accidental=0;
	if(root.length()>1){
		if(root[1]==L'b') accidental=-1;
		else if(root[1]==L'#') accidental=1;
	}
	switch(int(root[0])){
		case(int(L'A')):
			return 0x21+accidental;
		case(int(L'B')):
			return 0x23+accidental;
		case(int(L'C')):
			return 0x24+accidental;
		case(int(L'D')):
			return 0x26+accidental;
		case(int(L'E')):
			return 0x1c+accidental;
		case(int(L'F')):
			return 0x1d+accidental;
		case(int(L'G')):
			return 0x1f+accidental;
		default:
			return -0x1;
	}
}

void VSTHost::patternGenerationCallback(int populationcount, float mutationrate){
	//may store many of these arrays as a single array of structs, potential more efficient and/or more readable
	int tsnumerator=4;
	int mostfitindices[4], leastfitindices[4];
	bool stagnant=false;
	float targetfitness=0.125f;
	float*population[populationcount][4], fitnesses[populationcount][4], children[4][4][tsnumerator*drumpatternresolution];
	for(int x=0;x<populationcount;x++){
		for(int y=0;y<4;y++){
			population[x][y]=new float[tsnumerator*drumpatternresolution];
//			for(int z=0;z<tsnumerator*drumpatternresolution;z++) population[x][y][z]=parentdrumpattern[y][z];
			for(int z=0;z<tsnumerator*drumpatternresolution;z++) population[x][y][z]=rand()%2;
		}
	}
	for(int a=0;a<100;a++){
		std::cout<<"generation "<<a<<':'<<std::endl;
		for(int y=0;y<4;y++){
			mostfitindices[0]=0; mostfitindices[1]=1; mostfitindices[2]=2; mostfitindices[3]=3;
			leastfitindices[0]=0; leastfitindices[1]=1; leastfitindices[2]=2; leastfitindices[3]=3;
			for(int x=0;x<populationcount;x++){
				fitnesses[x][y]=0;
				for(int z=0;z<tsnumerator*drumpatternresolution;z++){
					fitnesses[x][y]+=float(!(population[x][y][z]==1.0f)!=!(parentdrumpattern[y][z]==1.0f));
//					std::cout<<"fitness["<<x<<"]["<<y<<"]["<<z<<"]="<<float(!(population[x][y][z]==1.0f)!=!(parentdrumpattern[y][z]==1.0f))<<std::endl;
				}
				fitnesses[x][y]/=tsnumerator*drumpatternresolution;
				fitnesses[x][y]=(fitnesses[x][y]==0)?0:(1/(targetfitness-fitnesses[x][y]));
//				std::cout<<"fitness["<<x<<"]["<<y<<"]="<<fitnesses[x][y]<<std::endl;
				//2 parents hard-coded here, adaptation for dynamic length doable, but not worth it rn
				//don't mistake 2 parents * 2 per pair, for 4 instruments; this 4 != the other 4
				//may combine below for loops later
				//doesn't prevent from duplicate items
				for(int u=0;u<4;u++){
					if(fitnesses[x][y]>fitnesses[mostfitindices[u]][y]){
						for(int v=2;v>u-1;v--){
							mostfitindices[v+1]=mostfitindices[v];
						}
						mostfitindices[u]=x;
						break;
					}
				}
				for(int u=0;u<4;u++){
					if(fitnesses[x][y]<fitnesses[leastfitindices[u]][y]){
						for(int v=2;v>u-1;v--){
							leastfitindices[v+1]=leastfitindices[v];
						}
						leastfitindices[u]=x;
						break;
					}
				}
			}
//			std::cout<<"least fit: "<<leastfitindices[0]<<", "<<leastfitindices[1]<<", "<<leastfitindices[2]<<", "<<leastfitindices[3]
//			<<"\nmost fit: "<<mostfitindices[0]<<", "<<mostfitindices[1]<<", "<<mostfitindices[2]<<", "<<mostfitindices[3]<<std::endl;
			//crossover occurs incluseive to crossover index (first float to be reversed is [crossoverindex]
			//could below for loops be combined and in the process eliminate some copy operations???
			int crossoverindex=rand()%(tsnumerator*drumpatternresolution);
			for(int u=0;u<tsnumerator*drumpatternresolution;u++){
				children[0][y][u]=population[mostfitindices[(u==crossoverindex)?1: 0]][y][u];
				children[1][y][u]=population[mostfitindices[(u==crossoverindex)?0: 1]][y][u];
				children[2][y][u]=population[mostfitindices[(u==crossoverindex)?2: 3]][y][u];
				children[3][y][u]=population[mostfitindices[(u==crossoverindex)?3: 2]][y][u];
			}
		}
		float mutationtemp;
		for(int y=0;y<4;y++){
			for(int x=0;x<tsnumerator*drumpatternresolution;x++){
				//how to mutate a float???
				//flipping individual bits could result in large-scale or miniscule value changes
				//adding random bounded value????
//				mutationtemp=((rand()%100)/100.0f<=mutationrate)?0.1f: 0.0f;
//				population[leastfitindices[0]][y][x]=children[0][y][x]+mutationtemp;
				population[leastfitindices[0]][y][x]=((rand()%100)/100.0f<mutationrate)?!bool(children[0][y][x]): children[0][y][x];
				population[leastfitindices[0]][y][x]=((rand()%100)/100.0f<mutationrate)?!bool(children[1][y][x]): children[1][y][x];
//				mutationtemp=((rand()%100)/100.0f<=mutationrate)?0.1f: 0.0f;
//				population[leastfitindices[2]][y][x]=children[2][y][x]+mutationtemp;
				population[leastfitindices[0]][y][x]=((rand()%100)/100.0f<mutationrate)?!bool(children[2][y][x]): children[2][y][x];
//				mutationtemp=((rand()%100)/100.0f<=mutationrate)?0.1f: 0.0f;
//				population[leastfitindices[3]][y][x]=children[3][y][x]+mutationtemp;
				population[leastfitindices[0]][y][x]=((rand()%100)/100.0f<mutationrate)?!bool(children[3][y][x]): children[3][y][x];
			}
//			std::cout<<std::endl;
		}
//		std::cout<<"\tfittest (0 -> 3):"<<std::endl;
//		for(int p=0;p<4;p++){
//			std::cout<<'\t';
//			for(int q=0;q<4;q++){
//				for(int r=0;r<tsnumerator*drumpatternresolution;r++){
//					std::cout<<population[mostfitindices[q]][p][r]<<' ';
//				}
//				std::cout<<"    ";
//			}
//			std::cout<<std::endl;
//		}
//		std::cout<<"\tfittestn't (0 -> 3):"<<std::endl;
//		for(int p=0;p<4;p++){
//			std::cout<<'\t';
//			for(int q=0;q<4;q++){
//				for(int r=0;r<tsnumerator*drumpatternresolution;r++){
//					std::cout<<population[leastfitindices[q]][p][r]<<' ';
//				}
//				std::cout<<"    ";
//			}
//			std::cout<<std::endl;
//		}
		stagnant=true;
	}
	for(int x=0;x<4;x++){
		for(int y=0;y<tsnumerator*drumpatternresolution;y++){
			//should i use last parent or last child?
			currentdrumpattern[x][y]=population[mostfitindices[0]][x][y];
		}
	}
}

void VSTHost::drumGeneticAlgorithm(int populationsize, float mutationrates[4]){
	int tsnumerator=4, nummatingpairs=10;
	int parentindices[nummatingpairs*2];
	std::vector<int> templateonsetindices;
	float population[populationsize][4][tsnumerator*drumpatternresolution],
			fitnessvalues[populationsize][4],
			fitnessprobabilities[populationsize][4],
			fitnessvaluetotal,
			children[nummatingpairs][4][tsnumerator*drumpatternresolution];

	memset(parentindices, 0, sizeof(parentindices));

	for(int x=0;x<populationsize;x++){
		for(int y=0;y<4;y++){
//			for(int z=0;z<tsnumerator*drumpatternresolution;z++) population[x][y][z]=rand()%2;
			for(int z=0;z<tsnumerator*drumpatternresolution;z++){
				population[x][y][z]=parentdrumpattern[y][z];
				if(x==0){
					if(parentdrumpattern[y][z]>0) templateonsetindices.push_back(z);
				}
			}
		}
	}

	for(int generation=0;generation<100;generation++){
//		std::cout<<"generation "<<generation<<": "<<std::endl;
		for(int sound=0;sound<4;sound++){
//			std::cout<<"\tsound "<<sound<<": "<<std::endl;
			fitnessvaluetotal=0.0f;
			for(int individual=0;individual<populationsize;individual++){
				fitnessvalues[individual][sound]=0.0f;
//				std::vector<int> onsetindices;
				for(int gene=0;gene<tsnumerator*drumpatternresolution;gene++){
//					onsetindices=std::vector<int>();
//					if(population[individual][sound][gene]>0) onsetindices.push_back(gene);
					fitnessvalues[individual][sound]+=abs(parentdrumpattern[sound][gene]-population[individual][sound][gene]);
				}
//				int minindex=0;
//				float minvalue=999.99f;
//				for(int index=0;index<(templateonsetindices.size()<onsetindices.size())?templateonsetindices.size():onsetindices.size();index++){
//					for(int compindex=0;compindex<(templateonsetindices.size()<onsetindices.size())?onsetindices.size():templateonsetindices.size();compindex++){
//						if(minvalue)
//					}
//				}
//				fitnessvalues[individual][sound]=pow(100, fitnessvalues[individual][sound]);
				fitnessvaluetotal+=fitnessvalues[individual][sound];
			}
			for(int individual=0;individual<populationsize;individual++){
				fitnessprobabilities[individual][sound]=fitnessvalues[individual][sound]/fitnessvaluetotal;
//				std::cout<<"fitness probability for individual "<<individual<<", sound "<<sound<<": "<<fitnessprobabilities[individual][sound]<<std::endl;
			}
			for(int parent=0;parent<nummatingpairs*2;parent++){
				float random=rand()/float(RAND_MAX), cumulativetotal=0.0f;
				for(int individual=0;individual<populationsize;individual++){
					cumulativetotal+=fitnessprobabilities[individual][sound];
					if(random<cumulativetotal){
						parentindices[parent]=individual;
						break;
					}
				}
			}
			for(int child=0;child<nummatingpairs;child++){
//				std::cout<<"\t\tparent "<<child*2<<": ";
//				for(int x=0;x<tsnumerator*drumpatternresolution;x++) std::cout<<population[parentindices[child*2]][sound][x]<<' ';
//				std::cout<<"fitness: "<<fitnessvalues[parentindices[child*2]][sound]<<" prob: "<<fitnessprobabilities[parentindices[child*2]][sound]<<"\n\t\tparent "<<child*2+1<<": ";
//				for(int x=0;x<tsnumerator*drumpatternresolution;x++) std::cout<<population[parentindices[child*2+1]][sound][x]<<' ';
				int crossoverpoint=rand()%(tsnumerator*drumpatternresolution);
//				std::cout<<"fitness: "<<fitnessvalues[parentindices[child*2+1]][sound]<<" prob: "<<fitnessprobabilities[parentindices[child*2+1]][sound]<<"\n\t\tcrossover point: "<<crossoverpoint<<std::endl;
//				std::cout<<"\t\tchild "<<child<<": ";
				for(int gene=0;gene<tsnumerator*drumpatternresolution;gene++){
					//crashed bad access here once
					children[child][sound][gene]=(gene<crossoverpoint)?(population[parentindices[2*child]][sound][gene]):(population[parentindices[2*child+1]][sound][gene]);
//					std::cout<<children[child][sound][gene]<<' ';
				}
//				std::cout<<std::endl;
			}
			int childcounter=0;
			bool leave;
			for(int individual=0;individual<populationsize;individual++){
				leave=false;
				for(int parent=0;parent<2*nummatingpairs;parent++){
					if(individual==parentindices[parent]) leave=true;
				}
				if(leave) break;
				for(int gene=0;gene<tsnumerator*drumpatternresolution;gene++){
					population[individual][sound][gene]=children[childcounter][sound][gene];
				}
				childcounter++;
				if(childcounter==nummatingpairs) break;
			}
			for(int mutationcount=0;mutationcount<int(populationsize*tsnumerator*drumpatternresolution*mutationrates[sound]);mutationcount++){
				int randomindividual=rand()%populationsize, randomgene=rand()%(tsnumerator*drumpatternresolution);
				population[randomindividual][sound][randomgene]=!bool(population[randomindividual][sound][randomgene]);
			}
		}
	}
	for(int x=0;x<4;x++){
		for(int y=0;y<tsnumerator*drumpatternresolution;y++){
			currentdrumpattern[x][y]=population[parentindices[0]][x][y];
		}
	}
}

void VSTHost::drumEventGenerationCallback(TimeData td, TimeData lasttd){
	float temptempo=160.0f;
	int temptsnum=4;
	//use midi offset param to adjust for time inaccuracy
	//could also use midi offset to humanize

	for(int x=0;x<4;x++){
		//edge only needs to be calculated once for all four sounds
		//inaccuracy available below, but how to adjust for it is difficult

		//could we use change in time (not beat or percent) to gauge offset?
		//this would be a much more authentic measure of time, independent of tempo or time sig

		//fundamentally, to correct for the offset, we need to lead the note because offset (i think) must be positive
		//we know that the loop is called every 0.01160997732 seconds (FRAMES_PER_BUFFER/SAMPLE_RATE)
		//therefore, we could make a temp TimeData, increment it by that amount, *then* do the edge callbackdata, then apply
		//proper midioffset

		//for now, i've decided not to mess with offset because even hard-coding it to work is iffy

		//below is temporary swing constant for testing
		float swing;

		bool edge=false;
		float beattotal=0.0f;
		if(td.beat!=lasttd.beat) edge=true;
		else{
			for(int x=1;x<drumpatternresolution;x++){
				swing=(x%2==0) ? 0.0f : 0.10f;
				if(td.percent>=100.0f*(float(x)/float(drumpatternresolution)+swing)
				   &&lasttd.percent<100.0f*(float(x)/float(drumpatternresolution)+swing)){
					edge=true;
					beattotal=float(x)/float(drumpatternresolution);
					break;
				}
			}
		}
		if(edge){
			beattotal+=td.beat;
			float temp=currentdrumpattern[x][int((beattotal-1)*drumpatternresolution)];
			this->addMidiEvent(drumsoundtokeymap.find(x)->second, temp);
			this->processE();
		}
	}
}

void VSTHost::loadParentDrumPattern(const char*xmlfilepath, int tsnumerator){
	for(int x=0;x<4;x++) parentdrumpattern[x]=nullptr;
	drumpatternresolution=0;
	std::ifstream xmlfile;
	xmlfile.open(xmlfilepath, std::ios::in);
	if(xmlfile.is_open()){
		bool closingtag, patternstag=false, resolutiontag=false, ridetag=false, hattag=false, snaretag=false, basstag=false, beattag=false, valuetag=false;
		TimeData beattimedatatemp={0, 0, 0, 0, 0, 0};
		float beattotal=0.0f;
		char buffer[256];
		int tagnum;
		while(xmlfile.get()=='<'){
			tagnum=-1;
			closingtag=false;
			if(xmlfile.peek()=='/'){
				closingtag=true;
				xmlfile.ignore();
			}
			xmlfile.get(buffer, 256, '>');
			xmlfile.ignore();
			if(std::strncmp(buffer, "patterns", 8)==0) patternstag=!closingtag;
			else if(std::strncmp(buffer, "resolution", 10)==0) resolutiontag=!closingtag;
			else if(std::strncmp(buffer, "ride", 4)==0) ridetag=!closingtag;
			else if(std::strncmp(buffer, "hat", 3)==0) hattag=!closingtag;
			else if(std::strncmp(buffer, "snare", 5)==0) snaretag=!closingtag;
			else if(std::strncmp(buffer, "bass", 4)==0) basstag=!closingtag;
			else if(std::strncmp(buffer, "beat", 4)==0) beattag=!closingtag;
			else if(std::strncmp(buffer, "value", 5)==0) valuetag=!closingtag;
			tagnum=ridetag?0: tagnum;
			tagnum=hattag?1: tagnum;
			tagnum=snaretag?2: tagnum;
			tagnum=basstag?3: tagnum;
			if(patternstag){
				if(resolutiontag){
					xmlfile.get(buffer, 256, '<');
					drumpatternresolution=std::stoi(buffer);
					for(int x=0;x<4;x++){
						parentdrumpattern[x]=new float[tsnumerator*drumpatternresolution];
						for(int y=0;y<tsnumerator*drumpatternresolution;y++) parentdrumpattern[x][y]=0.0f;
					}
				}
				if(tagnum!=-1){
					if(parentdrumpattern[0]==nullptr){
						std::cout<<"Parent drum pattern is nullptr, ensure that XML template defines resolution before "
				                    "defining the rhythm"<<std::endl;
						return;
					}
					if(beattag){
						xmlfile.get(buffer, 256, '.');
						xmlfile.ignore();
						beattotal=std::stof(buffer);
						xmlfile.get(buffer, 256, '<');
						beattotal+=std::stof(buffer)/100.0f;
					}
					if(valuetag){
						xmlfile.get(buffer, 256, '<');
//						std::cout<<"beattotal "<<beattotal<<", index "<<round((beattotal-1.0f)*float(drumpatternresolution))<<", value "<<std::stof(buffer)<<std::endl;
						parentdrumpattern[tagnum][int(round((beattotal-1.0f)*float(drumpatternresolution)))]=std::stof(buffer);
						beattotal=0.0f;
					}
				}
			}
			if(xmlfile.peek()=='\n') xmlfile.ignore();
			while(xmlfile.peek()==' ') {xmlfile.ignore();}
		}
	}
	else std::cout<<"xml file didnt open :/"<<std::endl;
	for(int x=0;x<4;x++){
		currentdrumpattern[x]=new float[tsnumerator*drumpatternresolution];
		for(int y=0;y<tsnumerator*drumpatternresolution;y++){
//			std::cout<<parentdrumpattern[x][y]<<"  ";
			currentdrumpattern[x][y]=parentdrumpattern[x][y];
		}
//		std::cout<<std::endl;
	}
}

void VSTHost::bassGeneticAlgorithm(std::vector<std::vector<Chord>> c, TimeData td, TimeData lasttd, VstTimeInfo ti, BassTonicMode tm){
	//temporary hard-definition of openness to experiment. current idea is float bounded [0.0f, 1.0f]
	float openness=0.5f;
	if(c[td.measure-1][td.beat-1].getRoot()!=L""){
		int totaldifference=0;
		if(currentchord.getFormatted()!=c[td.measure-1][td.beat-1].getFormatted()){
			currentchord=c[td.measure-1][td.beat-1];
			totaldifference=(td.measure-1)*ti.timeSigNumerator+td.beat-1;
			int beatindex=td.beat, measureindex=td.measure;
			for(;;){
				if(beatindex==ti.timeSigNumerator){
					beatindex=1;
					if(measureindex==c.size()) measureindex=1;
					else measureindex++;
				}
				else beatindex++;
				if(c[measureindex-1][beatindex-1].getRoot()!=L""){
					nextchord=c[measureindex-1][beatindex-1];
					totaldifference=((measureindex-1)*ti.timeSigNumerator+beatindex-1)-totaldifference;
					break;
				}
				if(measureindex==td.measure&&beatindex==td.beat){
					nextchord=currentchord;
					totaldifference=ti.timeSigNumerator*c.size();
					break;
				}
			}
			if(totaldifference<0) totaldifference+=ti.timeSigNumerator*c.size();
		}
		tonextchord=totaldifference;
		if(tonextchord<1) tonextchord+=c.size()*ti.timeSigNumerator;
		//E1, midi note 28 is lowest on 4-string bass (0x1c)
		//~E4, midi note 64 is ~highest on 4-string bass (0x40)
		char nexttonic=-0x1, currenttonic=rootToMidi(currentchord.getRoot());
//		std::wcout<<L"current tonic literal: "<<currentchord.getRoot()<<" current tonic midi note: "<<int(currenttonic)<<std::endl;
		//this system's functionality still needs to be checked
		if(tm==LOWEST) nexttonic=rootToMidi(nextchord.getRoot());
		else if(tm==DOWN){
			char tonictemp=rootToMidi(nextchord.getRoot());
			while(tonictemp-currenttonic<0) tonictemp+=12;
			nexttonic=tonictemp-12;
		}
		else if(tm==NEAREST){
			char tonictemp=rootToMidi(nextchord.getRoot());
			int distancetemp=999;
			//double check this lol
			while(distancetemp>abs(tonictemp-currenttonic)){
				tonictemp+=12;
				distancetemp=abs(tonictemp-currenttonic);
			}
			nexttonic=tonictemp-12;
		}
		else if(tm==UP){
			char tonictemp=rootToMidi(nextchord.getRoot());
			while(tonictemp-currenttonic<0) tonictemp+=12;
			nexttonic=tonictemp;
		}
		else if(tm==HIGHEST){
			nexttonic=rootToMidi(nextchord.getRoot())+12*3;
		}
//		std::wcout<<L"next tonic literal: "<<nextchord.getRoot()<<L" next tonic midi note: "<<int(nexttonic)<<std::endl;

		basssequence=std::map<TimeData, char>();
		basssequence.emplace(TimeData({0, 0, 0, 0, 0, 0}), currenttonic);

		int distance;
		for(int beat=0;beat<tonextchord+1;beat++){
			//okay so like weight potential distance to pick note from, and factor in distance to target???
			TimeData timestamp={(unsigned int)(beat/ti.timeSigNumerator), (unsigned int)(beat%ti.timeSigNumerator), 0, 0, 0, 0};
			distance=nexttonic-basssequence.find(timestamp)->second;
		}
//		std::cout<<"tonextchord: "<<tonextchord<<std::endl;
		basssequence=nextBassNote(currentchord, currenttonic, currenttonic, nexttonic, tonextchord-1, tonextchord-1);

		std::map<TimeData, char> tempsequence=std::map<TimeData, char>();
		for(auto&p:basssequence){
			tempsequence.emplace(p.first+td, p.second);
//			std::cout<<p.first<<" added to "<<td<<" to get "<<(p.first+td)<<std::endl;
		}
		basssequence=tempsequence;
//		basssequence.emplace(td, rootToMidi(currentchord.getRoot()));
	}
}
//perhaps recursion is not better than a loop in this scenario
std::map<TimeData, char> VSTHost::nextBassNote(Chord c, char startingnote, char currentnote, char targetnote, int beatstotarget, int totalbeats){
	//temporary hard-definition of openness to experiment. current idea is float bounded [0.0f, 1.0f]
	float openness=0.5f;
	std::map<TimeData, char> result, nextsequence;
	char nextnote;

	if(beatstotarget>-1){
		bool minor=false;
		bool dominantseven=false;
		for(auto&n:c.getNotes()){
			if(n==1.5) minor=true;
			if(n==5.0) dominantseven=true;
		}
		int scale[7];
//		for(int x=0;x<7;x++) scale[x]=minor?dorianscale[x]:(dominantseven?mixolydianscale[x]:ionianscale[x]);
		for(int x=0;x<7;x++) scale[x]=minor?dorianscale[x]:ionianscale[x];

		// (currentnote-21)%12 => 0=A, 1=A#, 2=B, etc.
		int index=-1;
		for(int x=0;x<7;x++){
			if((rootToMidi(c.getRoot())+scale[x])%12==(currentnote)%12){
				index=x;
				break;
			}
		}
//		if(index==-1) std::cout<<"currentnote "<<int(currentnote)<<" not in scale"<<std::endl;
//		else std::wcout<<"currentnote "<<int(currentnote)<<" determined to be degree "<<index<<" of "<<c.getRoot()<<(minor?" dorian":" ionian")<<" scale"<<std::endl;

		//note below lumps in =
		int directionmultiplier=(targetnote>currentnote)?1:-1;
		if(index==-1) nextnote=currentnote+1;
		else if(index<6) nextnote=currentnote+directionmultiplier*(scale[index+1]-scale[index]);
		else nextnote=currentnote+(scale[0]+12-scale[index]);
//		if(beatstotarget==1&&nextnote%12==targetnote%12) nextnote+=nextnote-currentnote;
		if(beatstotarget==1&&nextnote%12==targetnote%12) nextnote-=directionmultiplier;

//		std::cout<<"currentnote: "<<int(currentnote)<<" nextnote: "<<int(nextnote)<<" index: "<<index<<" scale offset: "<<scale[(index+1)%7]<<std::endl;

		//temporary hard-coded ti
		result.emplace(TimeData({(unsigned int)(totalbeats/4)-(unsigned int)(beatstotarget/4), (totalbeats%4)-(unsigned int)(beatstotarget%4), 0, 0, 0, 0}), currentnote);
		nextsequence=nextBassNote(c, startingnote, nextnote, targetnote, beatstotarget-1, totalbeats);
		result.insert(nextsequence.begin(), nextsequence.end());
	}

	return result;
}

void VSTHost::bassEventGenerationCallback(TimeData td, TimeData lasttd){
//	std::cout<<"time of call: "<<td<<std::endl;
	for(auto&p:basssequence){
//		std::cout<<"considering note "<<int(p.second)<<" @ timestamp "<<p.first<<std::endl;
		if((lasttd<p.first&&p.first<=td)||td==p.first){
//			std::cout<<"rising edge at timestamp "<<td.measure<<'.'<<td.beat<<'.'<<td.percent<<std::endl;
//			std::cout<<"note: "<<int(p.second)<<std::endl;
//			addMidiEvent(p.second, 1.0f);
			this->allMidiOff();
			this->addMidiEvent(p.second+12, 1.0f);
			this->processE();
//			std::cout<<"bass note processed: "<<int(p.second+12)<<std::endl;
			//below break assumes only one note @ a time
			break;
		}
	}
}

void VSTHost::pianoVoicingAlgorithm(Chord c){
	std::vector<std::vector<char>> candidates=std::vector<std::vector<char>>();
	for(int num=0;num<c.getNotes().size();num++){
		std::vector<char> temp=std::vector<char>();
		for(auto&n:c.getNotes()){

		}
	}
}

void VSTHost::getSamples(float**i, float**o){
	plugin->processReplacing(plugin, i, o, FRAMES_PER_BUFFER);
}

void VSTHost::processAdding(float**i, float**o){
	plugin->processReplacing(plugin, input, output, FRAMES_PER_BUFFER);
	int maxchannels=(plugin->numInputs>plugin->numOutputs)?plugin->numInputs: plugin->numOutputs;
	for(int sample=0;sample<FRAMES_PER_BUFFER;sample++){
		for(int channel=0;channel<maxchannels;channel++){
			if(channel<plugin->numInputs) i[channel%2][sample]+=input[channel][sample];
			if(channel<plugin->numOutputs) o[channel%2][sample]+=output[channel][sample];
		}
	}
}

int VSTHost::getNumInputs(){
	return plugin->numInputs;
}

int VSTHost::getNumOutputs(){
	return plugin->numOutputs;
}

std::wstring VSTHost::getDisplayString(){
	std::wstring out=L"";
	char name[256];
	plugin->dispatcher(plugin, effGetEffectName, 0, 0, name, 0);
	out+=L"Plugin: ";
	for(char c:name){
		if(c==L'\0') break;
		out+=wchar_t(c);
	}
	out+=L'\n'+std::to_wstring(plugin->numInputs)+L" in, "+std::to_wstring(plugin->numOutputs)+L" out\n";
	out+=L"Parameters:\n";
	char paramname[256], paramdisplay[256], paramlabel[256];
	for(int x=0;x<plugin->numParams;x++){
		plugin->dispatcher(plugin, effGetParamName, x, 0, paramname, 0);
		plugin->dispatcher(plugin, effGetParamDisplay, x, 0, paramdisplay, 0);
		plugin->dispatcher(plugin, effGetParamLabel, x, 0, paramlabel, 0);
		out+=L"   ";
		for(char c:paramname){
			if(c=='\0') break;
			out+=wchar_t(c);
		}
		out+=L": ";
		for(char c:paramdisplay){
			if(c==L'\0') break;
			out+=wchar_t(c);
		}
		for(char c:paramlabel){
			if(c==L'\0') break;
			out+=wchar_t(c);
		}
		out+=L'\n';
	}
	return out;
}

void VSTHost::addMidiEvent(char note, float velocity){
//	std::cout<<"midi note added: "<<int(note)<<", velocity: "<<velocity<<std::endl;
	float processedvelocity=(velocity>=0)?velocity:-velocity;
	processedvelocity=(processedvelocity>1)?1:processedvelocity;
	for(int x=0;x<me.size();x++){
		if(me[x]->midiData[1]==note) me.erase(me.begin()-1+x);
	}
	//may want to integrate channel functionality eventually
	me.push_back(new VstMidiEvent());
	me.back()->byteSize=sizeof(VstMidiEvent);
	me.back()->deltaFrames=0;
	me.back()->detune=0;
	me.back()->midiData[0]=(velocity>=0.0f)?char(0x91): char(0x81);
	me.back()->midiData[1]=note;
	me.back()->midiData[2]=processedvelocity*0x7f;
	me.back()->midiData[3]=0x0;
	me.back()->flags=kVstMidiEventIsRealtime;
	me.back()->noteLength=0;
	me.back()->noteOffset=0;
	me.back()->noteOffVelocity=0;
	me.back()->reserved1=0;
	me.back()->reserved2=0;
	me.back()->type=kVstMidiType;
	e->numEvents=me.size();
	e->reserved=0;
	for(int x=0;x<me.size();x++) e->events[x]=reinterpret_cast<VstEvent*>(me[x]);
//	std::cout<<"total notes: "<<me.size()<<std::endl;
}

void VSTHost::allMidiOff(){
	//all notes off message may exist! try 0x7b in first data byte (midiData[1] i think)
	std::vector<VstMidiEvent*> temp=me;
	me=std::vector<VstMidiEvent*>();
	for(int x=0;x<temp.size();x++){
		me.push_back(new VstMidiEvent());
		me.back()->byteSize=sizeof(VstMidiEvent);
		me.back()->deltaFrames=0;
		me.back()->detune=0;
		me.back()->midiData[0]=0x81;
		me.back()->midiData[1]=temp[x]->midiData[1];
		me.back()->midiData[2]=0xff;
		me.back()->midiData[3]=0x0;
		me.back()->flags=kVstMidiEventIsRealtime;
		me.back()->noteLength=0;
		me.back()->noteOffset=0;
		me.back()->noteOffVelocity=0;
		me.back()->reserved1=0;
		me.back()->reserved2=0;
		me.back()->type=kVstMidiType;
	}
//	for(int x=0;x<temp.size();x++) me.erase(me.begin());
	e->numEvents=me.size();
	e->reserved=0;
	for(int x=0;x<me.size();x++) e->events[x]=reinterpret_cast<VstEvent*>(me[x]);
	this->processE();
}

void VSTHost::processE(){
	plugin->dispatcher(plugin, effProcessEvents, 0, 0, e, 0.0f);
}