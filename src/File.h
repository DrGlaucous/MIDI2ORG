// Released under the MIT licence.
// See LICENCE.txt for details.

#pragma once

#include <stddef.h>
#include <stdio.h>

unsigned char* LoadFileToMemory(const char* file_path, size_t* file_size);
bool WriteFileFromMemory(const char* file_path, const unsigned char* memory_file, size_t file_size, const char* mode);

unsigned short File_ReadBE16(FILE* stream);
unsigned long File_ReadBE32(FILE* stream);
unsigned short File_ReadLE16(FILE* stream);
unsigned long File_ReadLE32(FILE* stream);
//unsigned long long File_ReadLE64(FILE* stream);//not needed ATM
unsigned char File_ReadLE8(FILE* stream);

void File_WriteBE16(unsigned short value, FILE* stream);
void File_WriteBE32(unsigned long value, FILE* stream);
void File_WriteLE16(unsigned short value, FILE* stream);
void File_WriteLE32(unsigned long value, FILE* stream);
//void File_WriteLE64(unsigned long long value, FILE* stream);//not needed ATM
void File_WriteLE8(unsigned char value, FILE* stream);
