## MIDI2ORG
`version 1.0.1` by Dr_Glaucous

[Find the source code here](https://github.com/DrGlaucous/MIDI2ORG)

This program allows the user to convert MIDI files into ORG files.
For those who are unaware, the organya (ORG) song format is used with the popular indie videogame ["Cave Story" (Doukutsu Monogatari)](https://en.wikipedia.org/wiki/Cave_Story) initially released in 2004. [You can download it here.](https://www.cavestory.org/download/cave-story.php)

It was developed by the game's creator, Daisuke "Pixel" Amaya specifically for said game.

Being as this music format is rather obscure, not many tools exist for altering it. This tool is now in that list.
___

### Program features:
* Drag and drop support for directory names
* Automatic and manual file simplification
* Drum channel handling
* Supports note volume conversion (only the initial volume of each note, though)
* Supports tempo conversion (once again, only the initial tempo)
* Supports time signature conversion
* That's about it... (for now)

&nbsp;
&nbsp;
&nbsp;

___
### Usage:

I tried my best to make the program as user-friendly as possible (that is, for a terminal application).

You simply just need to drag your MIDI file into the window and the program will present you with several
prompts for conversion.

**It is somewhat helpful to know basic MIDI structure when using this program.** The standard MIDI file has 16 channels, or "instruments" and an unlimited amount of tracks. Each track can use any of those 16 channels to play notes, though the notes each channel plays varies from track to track (I.E, channel 1 of track 2 may sound different from channel 1 of track 8). Channels are also not limited in the number of notes they are capable of playing at the same time. You can sit on the keyboard, and every last note will be recorded simultaneously.

The ORG format does not support this variety, so in order to capture as much of the MIDI data as possible, the MIDI2ORG program will create multiple orgs in a series of folders within the same directory as the MIDI that was converted.

Within the parent folder, there is 1 folder for each MIDI track, and anywhere from 0 to 16 ORGs inside representing the individual channels that track commands (Some MIDIs use track 0 as a conductor track, so no actual notes are played by it).

To combine these multiple files into one, use [ORGCopy](https://github.com/DrGlaucous/ORGCopy).
___
MIDI files are also **Much** more precise than ORGs. They can be 100s of times more fine in note placement than the former. Occasionally MIDI values share a common reduction factor, and values that were thousands long can be made into 100s or even 10s. The program automatically tries to find this reduction number, but more often than not, notes will be recorded in a way so that no common factor exists. In order to not lose any information, the program will use the values as-is, and the result is an ORG that is unreadable and very, *very* stretched out.

In order to combat this, MIDI2ORG will ask if the input requires **force-reduction**. The value of the shortest note the user wishes to accurately convey can be entered into the prompt, and the program will use that value to divide down the MIDI and round anything that doesn't get reduced evenly. **It is recommended to try auto-simplification first, and only resort to force-reduction if the result the first time was too big.** (Though in my experience, 90% of the time, force-reduction is required to get anything usable.)
___
MIDI2ORG can also process drum channels, but you have to tell it what channel the drums are on. The MIDI standard is to usually place the drums on the 10th channel. (actually channel **#9** when we include channel #0)

Drum-processed ORGs will divide the resultant notes into individual ORG tracks based on their note frequency. The actual instruments associated with each track will still have to be set manually.
___
#### Recommended Supplementary Tools

[ORGCopy](https://github.com/DrGlaucous/ORGCopy) - the companion tool to MIDI2ORG, invaluable in its ability to turn multiple orgs containing 1 channel to a single ORG containing multiple channels.

[Musescore](https://musescore.org/) - good for determining the shortest notes of the song in terms of standard music notes

[MidiEditor](https://www.midieditor.org/) - good for determining the relationship of the channels to the tracks and finding any drum channels

[ORGMaker2](https://www.cavestory.org/download/music-tools.php) - the program used to actually **view and edit** .org files [-and it's also open-source, too](https://github.com/shbow/organya)

___
### Building:
In the same directory as the CMakeLists.txt, enter the following:

Generate Makefiles:
`cmake -B ./build`

(append >`-G"MSYS Makefiles"`< if using MSYS2 to build)


Generate executable:
`cmake --build ./build --config Release`

The final executable can be found in the "bin" directory

___
### Changelog:
#### Version 1.0.2
* fixed several potential crashes when entering large force-simplification values
* fixed the issue where fast songs could receive a "0" tempo (this crashes ORGMaker)
* updated the method of handling notes (too high or low) out of ORG range
* parser can now handle dirtier MIDIs (specifically if there are "note.off" events for notes that were never "on" in the first place)
* fixed out-of-order events causing the parser to put a note in the next track when it didn't need to
#### Version 1.0.1
* support for all 3 binary MIDI types
* drum track handling
* auto and manual length simplification

___
### Credits:
Organya Music Format: Daisuke "Pixel" Amaya

"File.cpp" (mostly borrowed from CSE2, a reverse-engineering of the cave story engine): Clownacy and whoever else...

[MIDI-Parser](https://github.com/MStefan99/Midi-Parser) library: MStefan99

Everything left over: Dr_Glaucous