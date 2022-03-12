#pragma once

#include <stddef.h>
#include <stdio.h>


#ifndef NULL
	#ifdef __cplusplus
		#define NULL 0
	#else
		#define NULL ((void *)0)
	#endif
#endif

#define MAXTRACK 16//max tracks in an ORG file (1-8 and Q-something or other)
#define MAXCHANNEL 16//MIDI's max supported insturment channels (each track can have this ammount)

//ORG typedefs
typedef struct TRACKINFO
{
	size_t note_num;


} TRACKINFO; 

//ORG typedefs
typedef struct ORGFILES
{
	TRACKINFO tracks[MAXTRACK];
}ORGFILES;

//data that we glean from the MIDI file
typedef struct NOTEDATA
{
	int TimeStart{};
	int TimeRelative{};
	//int Length;
	int Pitch{};
	int Channel{};
	//int Pan;
	char Volume{};
	bool Status{}; //on/off
}NOTEDATA;

//data that we push to the ORG file
typedef struct ORGNOTEDATA
{
	int TimeStart{};
	int Length{};
	unsigned char Pitch{};
	unsigned char Volume{};
	bool LengthSet{};//tell us if we found the stopping point of this note
	bool EventType{};//true == start, false == stop

}ORGNOTEDATA;