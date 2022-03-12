// To convert binary MIDIs to Organya files
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <iterator>
#if __cplusplus < 201703L // If the version of C++ is less than 17
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING//silence. 
#include <experimental/filesystem>
	// It was still in the experimental:: namespace
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif


#include "MIDI2ORG.h"
#include "File.h"
#include "Midi.h"

#define PRGMVERSION "1.0.1"
#define READ_LE16(p) ((p[1] << 8) | p[0]); p += 2
#define READ_LE32(p) ((p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0]); p += 4


TRACKINFO tracks[MAXTRACK];
ORGFILES orgs[2];
//static unsigned int AbsTime{};//absolute time since the start of the file
static unsigned short gTempo{};

//used to determine the new beat structure of the song
int gcd(int a, int b) {
	//gcd: greatest common difference
	if (b == 0)
		return a;
	return gcd(b, a % b);
}
int LeastCommonMultiple(int num1, int num2)
{
	return (num1 * num2) / gcd(num1, num2);

}
int gcdArray(std::vector<int> processArray)
{
	//catch any 0 length arrays sent our way
	if (processArray.size() == 0)
	{
		return 1;//you can't devide by 0, now can you?
	}

	int result = *processArray.begin();
	for (int i = 0; i < processArray.size(); ++i)
	{
		result = gcd(*(processArray.begin() + i), result);

		if (result == 1)
		{
			return result;
		}
	}
	return result;
}

//change the timing of a song
void StretchSong(unsigned char *memfile, char bpmStretch, char dotStretch)
{

	//takes the signature and adjusts it accordingly
	memfile[8] = memfile[8] * bpmStretch;
	memfile[9] = memfile[9] * dotStretch;

	//adjusts tempo (bytes 6 and 7)
	short newTempo = (memfile[7] << 8) | memfile[6];//read data into our short
	newTempo = newTempo / (bpmStretch * dotStretch);//set the tempo relative to the stretch value
	for (unsigned int i = 0; i < 2; ++i)//push the updated values back to the data
		memfile[i + 6] = newTempo >> (8 * i);


	//we need to change the X value of all the notes and the leingth of all the notes
	//we need to use note_num to determine the note ammount in each track



	memfile += 18;//end of header data
	//iterate through all tracks and get the number of notes in each one
	for (int i = 0; i < MAXTRACK; i++)
	{
		memfile +=  4;//jump to note_num value

		tracks[i].note_num = READ_LE16(memfile);
	}

	//iterate through all tracks to append new note values
	for (int i = 0; i < MAXTRACK; i++)
	{

		//this makes the new x values
		for (int j = 0; j < tracks[i].note_num; ++j)//for each note
		{

			int writeOut = READ_LE32(memfile);//get x value and multiply it by the total stretch value
			writeOut *= (bpmStretch * dotStretch);

			memfile -= 4;//undo the previous advance

			for (unsigned int i = 0; i < 4; ++i)//write to the memfile
				memfile[i] = writeOut >> (8 * i);

			memfile += 4;//advance to next value


		}

		memfile += tracks[i].note_num;//jump over Y value and go to length value

		//this makes new length values
		for (int j = 0; j < tracks[i].note_num; ++j)//for each note
		{
			char writeOut = memfile[0];//get the value of the leingth
			writeOut *= (bpmStretch * dotStretch);

			memfile[0] = writeOut;
			++memfile;//advance to next number


		}

		memfile += (tracks[i].note_num * 2);//skip to end of data
	}

}

//checks (and removes) any "s or 's around the file path
void CheckForQuote(std::string *inpath)
{
	std::string path = *inpath;

	//check to see if the path is enclosed in " or '
	if (path.size() > 0)
	{
		if (
			((*path.begin() == '\"') &&
				(*(path.end() - 1) == '\"')) ||
			((*path.begin() == '\'') &&
				(*(path.end() - 1) == '\''))
			)
		{
			path = path.substr(1, path.size() - 2);//remove the enclosing ""s from the input
		}
	}

	*inpath = path;


}

char pass[5] = "MThd";//midi header

//tells if the file is an MIDI or not
int VerifyFile(const char* path)
{



	FILE* file = fopen(path, "rb");
	
	if(file == NULL)
	{
		return -1;//file does not exist
	}

	char header[4];
	
	fread(header, 1, 4, file);//get header

	if (memcmp(header, pass, 4) != 0)//see if the header matches either orgv1 or orgv2
	{
		fclose(file);
		return 1;//file does not have MIDI header
	}

	fclose(file);

	return 0;//success
}

bool ConvertMidi(const char* path)
{

	NOTEDATA PrepNote;
	std::string DirectoryPath = path + std::string(".fold");

	if (!(fs::is_directory(DirectoryPath.c_str())))//checks to see if the directory exists, skips making it if this is true
	{
		if (!(fs::create_directories(DirectoryPath.c_str())))
		{
			std::cout << "Error! Could not create directory:\n" 
				<< DirectoryPath.c_str()
				<< std::endl;
			return false;
		}
	}

	//smf::MidiFile MIDIFILE;

	//MIDIFILE.read(path);

	Midi MIDIFile{ path };

	auto& header = MIDIFile.getHeader();
	auto& tracks = MIDIFile.getTracks();
	//channel

	std::cout << "Track Count: "<<header.getNTracks() << std::endl;



	//I think tempos can be directly ported from MIDI to ORG (same type of thing)
	gTempo = header.getDivision();

	int currTrack = 0;//use this to know what track we are looking at
	for (const auto& track : tracks)//iterates through all tracks
	{

		unsigned int AbsTime = 0;//absolute time since the start of the track
		std::vector<NOTEDATA> TrackData[MAXCHANNEL];//one vector for each channel (will be reused per each track)


		auto& events = track.getEvents();

		for (const auto& trackEvent : events)//iterates through all track events
		{
			auto* event = trackEvent.getEvent();
			//uint8_t* data;

			if (event->getType() == MidiType::EventType::MidiEvent)//I believe metaEvents contain the tempo and key signature info
			{
				auto* midiEvent = (MidiEvent*)event;
				auto status = midiEvent->getStatus();

				/*
				switch (status)
				{
				case MidiType::MidiMessageStatus::NoteOn:
				case MidiType::MidiMessageStatus::NoteOff:
				}
				*/

				if (status == MidiType::MidiMessageStatus::NoteOn)
				{

					//velocity == loudness (0 is note Off)
					//channel == location (we will have 1 folder for each track, and 1 org for each used channel)
					//frequency == note pitch
					PrepNote.Volume = (char)(midiEvent->getVelocity());
					PrepNote.Pitch = midiEvent->getNote();
					PrepNote.Channel = midiEvent->getChannel();
					PrepNote.Status = (midiEvent->getVelocity() ? true : false);
					PrepNote.TimeRelative = trackEvent.getDeltaTime().getData();
					AbsTime += PrepNote.TimeRelative;
					PrepNote.TimeStart = AbsTime;
					TrackData[PrepNote.Channel].push_back(PrepNote);//record the data to our vector

					std::cout << "Loudness " << (int)PrepNote.Volume << std::endl;
					std::cout << "Frequency " << (int)PrepNote.Pitch << std::endl;
					std::cout << "Channel " << (int)PrepNote.Channel;

					std::cout << "\t\t\tLength: " << PrepNote.TimeRelative
							<< "\t\tABSLength: " << AbsTime << std::endl;


				}
				else if (status == MidiType::MidiMessageStatus::NoteOff)
				{
					PrepNote.Volume = (char)(midiEvent->getVelocity());
					PrepNote.Pitch = midiEvent->getNote();
					PrepNote.Channel = midiEvent->getChannel();
					PrepNote.Status = false;
					PrepNote.TimeRelative = trackEvent.getDeltaTime().getData();
					AbsTime += PrepNote.TimeRelative;
					PrepNote.TimeStart = AbsTime;
					TrackData[PrepNote.Channel].push_back(PrepNote);//record the data to our vector


					std::cout << "\t\t\tLength: " << PrepNote.TimeRelative << std::endl;
					std::cout << "\t\t\tABSLength: " << AbsTime << std::endl;
				}



			}
			else
			{
				std::cout << "\t\t\tLength: " << trackEvent.getDeltaTime().getData() << std::endl;
			}


		}


		//NOTE: use Musescore to fix up the midi if you recored it: It will automatically snap notes to standard lengths so 
		//the size of the addresses can be reduced even more (does it, though? I need to test it myself).
		//
		std::vector<int> ArrayOfLength;//will be used to find the gcd of all the MIDI events (so we can hopefully shrink those values some)
		//see if the file can be reduced
		for (int i = 0; i < MAXCHANNEL; ++i)//for each vector in the array
		{

			//catch any 0 length arrays sent our way
			if (TrackData[i].size() == 0)
			{
				continue;
			}

			//int ZeroValue = (TrackData[i].begin()->TimeStart) - (TrackData[i].begin()->TimeRelative);//in case we had to use the absolute time, this allows the start value to be anything (but we can just use relative time)
			for (int j = 0; j < TrackData[i].size(); ++j)//for each event recorded in the track
			{

				if(((TrackData[i].begin() + j)->TimeRelative) == 0)
				{
					continue;//disregard any 0 addresses
				}

				ArrayOfLength.push_back(((TrackData[i].begin() + j)->TimeRelative));//add the event times to the vector
			}



		}

		int reductionRate = gcdArray(ArrayOfLength);
		std::cout << "Shrinkable by: " << reductionRate << std::endl;//what space can we save?


		//force compression: get an input as to how much we want the song crunched down
		//This is good for cases where a few little notes will balloon up the entire track, or if the midi was created
		//with imperfect locations. 
		//deviding an integer will produce an integer + remainder,
		//we will reound the notes forward or backward depending on how much is left in said remainder (big values will round up, small ones down)

		//compression here
		for (int i = 0; i < MAXCHANNEL; ++i)//for each vector in the array
		{
			for (int j = 0; j < TrackData[i].size(); ++j)//for each event recorded in the track
			{
				//compress addresses
				(TrackData[i].begin() + j)->TimeRelative = ((TrackData[i].begin() + j)->TimeRelative) / reductionRate;
				(TrackData[i].begin() + j)->TimeStart = ((TrackData[i].begin() + j)->TimeStart) / reductionRate;
			}
		}










		//this is where we need to put the data from our vector into a file
		std::string TrackSubfolder = DirectoryPath + '/' + std::to_string(currTrack) + ".fold";


		if (!(fs::is_directory(TrackSubfolder.c_str())))//checks to see if the directory exists, skips making it if this is true
		{
			if (!(fs::create_directories(TrackSubfolder.c_str())))
			{
				std::cout << "Error! Could not create directory:\n"
					<< TrackSubfolder.c_str()
					<< std::endl;
				return false;
			}
		}


		for (int i = 0; i < MAXCHANNEL; ++i)//go through all channels of notes (one ORG file will be made per each channel)
		{
			if (TrackData[i].size() == 0)//skips the track if there are no note events in there
			{
				continue;
			}

			std::vector<ORGNOTEDATA> TrackDataOrg[MAXTRACK];//One vector per each ORG track (used to bump notes to the next track if the current note is still active)
			ORGNOTEDATA BufferNote;


			for (int z = 0; z < TrackData[i].size(); ++z)//for every MIDI note event
			{
			

				BufferNote.Pitch = (TrackData[i].begin() + z)->Pitch - 12;//middle C in MIDI is 60, in ORG it is 48, a difference of 12
				BufferNote.TimeStart = (TrackData[i].begin() + z)->TimeStart;//no conversions needed here
				BufferNote.Volume = 30;//lets make this 30 for now (we will use initial velocity from the MIDI later)
				BufferNote.Length = 1;//in case the MIDI never tells this note to stop, it will still have a termination point
				BufferNote.LengthSet = false;//tells us if we need to move on to the next track or not (true == good to add another note behind it)
				BufferNote.EventType = (TrackData[i].begin() + z)->Status;//start/stop
	
				//iterate through the ORG tracks to ask the questions below
				for (int j = 0; j < MAXTRACK; ++j)
				{

					//see the MIDI event's status: if it is an OFF note event, check and see what track has that note as the last one played
					//we may be pushing back multiple OFF events

					if (TrackDataOrg[j].size() == 0)//automatically insert the buffer since there is nothing before it
					{
						TrackDataOrg[j].push_back(BufferNote);
						break;//get out of this for loop: we gave the data a home.
					}



					//we need to put this one above the condition seen above (because if we are playing 2 of the same notes, we want them to be separate)
					//we also need to check if the input is a start function before doing this \/
					if ((TrackDataOrg[j].end() - 1)->LengthSet == true && BufferNote.EventType == true)//check to see if the previous note has stopped playing
					{
						TrackDataOrg[j].push_back(BufferNote);//we are allowed to push the next note back
						break;//data applied successfully

					}


					//the only issue I see here is if we get two note.start commands for the same note without any note.stops in between (this is not likely to happen, but I think this assumption may come back to bite me)
					//no need to check if the type is a startNote because all conditions where that would be true are caught in the statement above
					if ((TrackDataOrg[j].end() - 1)->Pitch == BufferNote.Pitch)//are this and last note the same
					{



						(TrackDataOrg[j].end() - 1)->Length = (BufferNote.TimeStart) - ((TrackDataOrg[j].end() - 1)->TimeStart);//delta time
						(TrackDataOrg[j].end() - 1)->LengthSet = true;//note is complete

						//break-up function
						if((TrackDataOrg[j].end() - 1)->Length > 255)//ORG notes can only be 1 char long, so any longer than that will need to be cut up into multiple notes
						{

							BufferNote = *(TrackDataOrg[j].end() - 1);//copy the last note to the buffer
							BufferNote.Length = 255;

							int OfTotalLength = (TrackDataOrg[j].end() - 1)->Length;//used to see how much of the total note length we have left

							TrackDataOrg[j].pop_back();//remove the note whose data is too long

							while (OfTotalLength > 255)//keep pushing back clones of the note (but only 255 in length) until our total length is less than 1 char
							{

								TrackDataOrg[j].push_back(BufferNote);

								BufferNote.TimeStart += 255;
								OfTotalLength -= 255;
							}
							BufferNote.Length = OfTotalLength;
							TrackDataOrg[j].push_back(BufferNote);

						}


						break;//data applied successfully

					}

				}
	
				//if we get here and were not able to find a space, the note is (unfortunately) discarded
	
				//check: is previous note the same?
			// if no, check if the previous note's deltaTime is set
			// if no, bump to the next track and repeat
			// 
			// if the note IS the same, regardless of status, we can slap it on (it will simply change the note already on the board)
			// to slap it on, take deltaTime between the notes and set the previous note's deltaTime variable
			// 
			//process: send note data
	
			}

			//time to make the org file

			std::string ORGPath = TrackSubfolder + std::string("/Channel") + std::to_string(i) + ".org";

			FILE* outFile = fopen(ORGPath.c_str(), "wb");

			if (outFile == NULL)
			{
				std::cout << "Error! Could not create file:\n"
					<< ORGPath.c_str()
					<< std::endl;
				fclose(outFile);
				return false;
			}

			//setup header data

			char BeatsPM = 4;//later, we may try to get this data from the MIDI
			char NotesPB = 4;

			fwrite("Org-02", 6, 1, outFile);
			//fwrite(&gTempo, 2, 1, outFile);
			File_WriteLE16(gTempo, outFile);//this allows us to run this code no matter what endian the host computer is
			File_WriteLE8(BeatsPM, outFile);
			File_WriteLE8(NotesPB, outFile);
			File_WriteLE32(0, outFile);//start of repeat loop
			File_WriteLE32(1000, outFile);//end of repeat loop
			for (int j = 0; j < MAXTRACK; ++j)//write note info
			{
				
				File_WriteLE16(1000, outFile);//insturment frequency, default is 1000
				File_WriteLE8(0, outFile);//insturment, will make them all 0
				File_WriteLE8(0, outFile);//Pipi, 0 by default
				File_WriteLE16(TrackDataOrg[j].size(), outFile);//number of notes in this track

			}

			for (int j = 0; j < MAXTRACK; ++j)//write the notes themselves (do it for each track)
			{

				//iterate through each note
				for (int z = 0; z < TrackDataOrg[j].size(); ++z)
					File_WriteLE32((TrackDataOrg[j].begin() + z)->TimeStart, outFile);//X value
				for (int z = 0; z < TrackDataOrg[j].size(); ++z)
					File_WriteLE8((TrackDataOrg[j].begin() + z)->Pitch, outFile);//Y location
				for (int z = 0; z < TrackDataOrg[j].size(); ++z)
					File_WriteLE8((unsigned char)(TrackDataOrg[j].begin() + z)->Length, outFile);//length (we need to greatly reduce the size of the org tempo for the actual length to fit here (I.E run the values through the gcm function to get the devisor))
				for (int z = 0; z < TrackDataOrg[j].size(); ++z)
					File_WriteLE8((TrackDataOrg[j].begin() + z)->Volume, outFile);//volume
				for (int z = 0; z < TrackDataOrg[j].size(); ++z)
					File_WriteLE8(6, outFile);//PAN (I need to grab this from the MIDI, too, kind of like the key signature)

			}


			fclose(outFile);

		}



		currTrack += 1;

	}





	return false;

}



int main(void)
{

	std::string InputText;
	std::string Path1;

	char confirm = 0;


	std::cout << "MIDI2ORG by Dr_Glaucous (2022) version: " << PRGMVERSION << std::endl;

	bool canExit = false;
	while (canExit == false)
	{

		//New interface pattern:
		//Drag midi in:
		//Does the midi have a drum track?
		//	If yes, what track is it (typically track 10)
		//Do you want to force simplification?
		//	if yes, enter the division factor (best to make it the length of the shortest note [enter something like 32 for a 32nd note, we can use song tempo to determine the size that will be]), or do automatic (find the shortest note length and set that to 1)
		//
		//program will create a folder containing ORGs in whatever directory the MIDI is in (or should it do it in the current directory)



		//get the directory for the MIDI
		
		bool ValidFirstOrg = false;
		while (ValidFirstOrg == false)
		{
			std::cout << "Please enter the MIDI to be converted." << std::endl;
			getline(std::cin, InputText);
			CheckForQuote(&InputText);


			switch (VerifyFile(InputText.c_str()))
			{
			case -1:
				std::cout << "Error: File does not exist. Please try again." << std::endl;
				break;
			case 0:
				ValidFirstOrg = true;
				Path1 = InputText;
				break;
			case 1:
				std::cout << "What you entered may not be a MIDI!\nDo you still wish to proceed? (y/n)" << std::endl;
				std::cin >> confirm;
				if (tolower(confirm) == 'y')//the user wishes to continue
				{
					confirm = 0;//reset confirm
					ValidFirstOrg = true;
					Path1 = InputText;
				}
				break;
			}



		}
		

		//TEST to see if gcdArray works (it does)
		/*
		std::vector<int> gcfTest;
		int argCount = 0;
		while (argCount < 10)
		{
			std::cout << "Input number: " << argCount + 1 << std::endl;
			int inn;
			std::cin >> inn;
			gcfTest.push_back(inn);
			++argCount;
		}
		argCount = 0;

		std::cout << gcdArray(gcfTest) << std::endl;
		*/



		std::cout << "\nReady? (y/n)\n";
		std::cin >> confirm;
		if (tolower(confirm) == 'y')//the user wishes to continue
		{


			confirm = 0;//reset confirm
			ConvertMidi(Path1.c_str());

		}





	}
	
	std::cout << "\nProcess Finished.\nFeel free to close the window or enter any value to exit." << std::endl;

	char isFinished;//this is the best and most universal way I can find to halt the terminal for user input
	std::cin >> isFinished;


	return 0;
}
