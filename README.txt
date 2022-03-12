ORGCopy version 1.0.1 by Dr_Glaucous
This simple program allows the user to copy tracks from one Organya song to another.
For those who are unaware, the organya song format is used with the popular indie videogame "Cave Story" (Doukutsu Monogatari) initally released in 2004.
It was developed by the game's creator, Daisuke "Pixel" Amaya specifically for that purpose.


Program features:
Drag and drop support for directory names
Automatic key signature adjustment to account for any differences between songs
That's about it...




to build:

Generate Makefiles:
cmake -B ./build

(append >-G"MSYS Makefiles"< if using MSYS2 to build)


Generate executable:
cmake --build ./build --config Release

final executable can be found in the "bin" directory
