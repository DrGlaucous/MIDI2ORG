cmake_minimum_required(VERSION 3.10)


# set the project name
project(MIDI2ORG VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#options
option(STATIC_CRT "Use static CRT libraries" ON)


# add the executable
add_executable(MIDI2ORG
	"src/MIDI2ORG.cpp"
	"src/MIDI2ORG.h"
	"src/File.cpp"
	"src/File.h"


	#"lib/midifile/include/Binasc.h"
	#"lib/midifile/include/MidiEvent.h"
	#"lib/midifile/include/MidiEventList.h"
	#"lib/midifile/include/MidiFile.h"
	#"lib/midifile/include/MidiMessage.h"
	#"lib/midifile/include/Options.h"

	#"lib/midifile/src/Binasc.cpp"
	#"lib/midifile/src/MidiEvent.cpp"
	#"lib/midifile/src/MidiEventList.cpp"
	#"lib/midifile/src/MidiFile.cpp"
	#"lib/midifile/src/MidiMessage.cpp"
	#"lib/midifile/src/Options.cpp"


)

#add the midi parsing library
message(STATUS "adding midifile")
add_subdirectory("lib/MIDIParse")
target_link_libraries(MIDI2ORG PRIVATE libParser)


# put the final executable in the bin directory for ease of access
set_target_properties(MIDI2ORG PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

# make the libraries statically linked
target_link_options(MIDI2ORG PRIVATE "-static")


# MSVC changes
if(MSVC)
	# remove the warnings when using non-"_s" functions
	target_compile_definitions(MIDI2ORG PRIVATE _CRT_SECURE_NO_WARNINGS)

	# source files are recongnized as /utf-8
	target_compile_options(MIDI2ORG PRIVATE "/utf-8")

	# statically-link the CRT (so MSVC dlls are not required to run the program)
	#this currently causes buld issues to happen, so I'm commenting it out.
	foreach(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif()
	endforeach()


endif()

