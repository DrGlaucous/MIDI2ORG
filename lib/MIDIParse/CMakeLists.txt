#a message for my convenience
message(STATUS "adding MIDIParse")

add_library(libParser)
target_sources(libParser PRIVATE
		ReadNumber.cpp
		VLQ.cpp
		Event.cpp
		MidiEvent.cpp
		SysExEvent.cpp
		MetaEvent.cpp
		MTrkEvent.cpp
		BaseChunk.cpp
		HeaderChunk.cpp
		TrackChunk.cpp
		Midi.cpp
		)



# MSVC changes
if(MSVC)
	# remove the warnings when using non-"_s" functions
	target_compile_definitions(libParser PRIVATE _CRT_SECURE_NO_WARNINGS)

	# source files are recongnized as /utf-8
	target_compile_options(libParser PRIVATE "/utf-8")

	# statically-link the CRT (so MSVC dlls are not required to run the program)
	#this currently causes buld issues to happen, so I'm commenting it out.
	foreach(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif()
	endforeach()
endif()

target_include_directories(libParser PUBLIC .)

