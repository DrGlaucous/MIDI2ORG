// "File" pulled from CSE2
// Released under the MIT licence.
// See LICENCE.txt for details.

#include "File.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

unsigned char* LoadFileToMemory(const char* file_path, size_t* file_size)
{
	unsigned char* buffer = NULL;

	FILE* file = fopen(file_path, "rb");

	if (file != NULL)
	{
		if (!fseek(file, 0, SEEK_END))
		{
			const long _file_size = ftell(file);

			if (_file_size >= 0)
			{
				rewind(file);
				buffer = (unsigned char*)malloc(_file_size);

				if (buffer != NULL)
				{
					if (fread(buffer, _file_size, 1, file) == 1)
					{
						fclose(file);
						*file_size = (size_t)_file_size;
						return buffer;
					}

					free(buffer);
				}
			}
		}

		fclose(file);
	}

	return NULL;
}

//writes back in the same manner that the function above loads from memory, not used ATM (I'm keeping it around in case I need it, though)
bool WriteFileFromMemory(const char* file_path, const unsigned char* memory_file, size_t file_size, const char* mode)
{
	FILE* file = fopen(file_path, mode);

	if (file == NULL)
	{
		return false;
	}

	fwrite(memory_file, file_size, 1, file);

	return true;

}

//returns a pointer to a file that has the corresponding chunk taken out
/*
unsigned char* DeleteByteSection(unsigned char* fileInMemory, size_t file_size, int deleteAddress, int deleteLeingth)
{

	if (deleteAddress + deleteLeingth > file_size)
	{
		return NULL;
	}

	unsigned char* buffer = NULL;

	buffer = (unsigned char*)malloc(file_size);

	memcpy(buffer, fileInMemory, deleteAddress);//copies everything up to that point
	memcpy(buffer + deleteAddress, fileInMemory + deleteAddress + deleteLeingth, file_size - (deleteAddress + deleteLeingth));//copies everything after that point

	return buffer;

}
*/

unsigned short File_ReadBE16(FILE* stream)
{
	unsigned char bytes[2];

	fread(bytes, 2, 1, stream);

	return (bytes[0] << 8) | bytes[1];
}

unsigned long File_ReadBE32(FILE* stream)
{
	unsigned char bytes[4];

	fread(bytes, 4, 1, stream);

	return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

unsigned short File_ReadLE16(FILE* stream)
{
	unsigned char bytes[2];

	fread(bytes, 2, 1, stream);

	return (bytes[1] << 8) | bytes[0];
}

unsigned long File_ReadLE32(FILE* stream)
{
	unsigned char bytes[4];

	fread(bytes, 4, 1, stream);

	return (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
}

/*
//after-market (I want a variable that is 8 bytes, yet long long is still giving me integer overflow warnings)
unsigned long long File_ReadLE64(FILE* stream)
{
	unsigned char bytes[8];//number of chunks to read

	fread(bytes, 8, 1, stream);//read 1 chunk with 8 bytes

	return (bytes[7] << 56) | (bytes[6] << 48) | (bytes[5] << 40) | (bytes[4] << 32) | (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];//appends each chunk read to the variable in the correct order (here, front to back)
}
*/

//after-market
unsigned char File_ReadLE8(FILE* stream)
{
	unsigned char bytes[1];

	fread(bytes, 1, 1, stream);

	return bytes[0];
}

void File_WriteBE16(unsigned short value, FILE* stream)
{
	for (unsigned int i = 2; i-- != 0;)
		fputc(value >> (8 * i), stream);
}

//BE32 writes from the back forward
void File_WriteBE32(unsigned long value, FILE* stream)
{
	for (unsigned int i = 4; i-- != 0;)
		fputc(value >> (8 * i), stream);
}

//writes front backward, using 16 bits (2 chunks of 8)
void File_WriteLE16(unsigned short value, FILE* stream)
{
	for (unsigned int i = 0; i < 2; ++i)
		fputc(value >> (8 * i), stream);
}

//LE32 writes from the front backward, using 32 bits (4 chunks of 8)
void File_WriteLE32(unsigned long value, FILE* stream)
{
	for (unsigned int i = 0; i < 4; ++i)
		fputc(value >> (8 * i), stream);//this shifts the value of "value" to the right (8 bits * i), it essentially devides the value up into byte sized chunks so that an unsigned long can fit into 4 bytes
		//the shift happens in binary, for instance:
		//
		//foo = 010001
		//foo >> 3 = 000010
		//
}

/*
//LE64 writes from the front backward, using 64 bits (8 chunks of 8) [writes <---small place value-------big place value-  ]
void File_WriteLE64(unsigned long long value, FILE* stream)
{
	for (unsigned int i = 0; i < 8; ++i)
		fputc(value >> (8 * i), stream);//this shifts the value of "value" to the right (8 bits * i), it essentially devides the value up into byte sized chunks so that an unsigned long can fit into 4 bytes

}
*/

//writes front backward, using 8 bits (1 chunk of 8)
void File_WriteLE8(unsigned char value, FILE* stream)
{
	fputc(value, stream);//because a char is only 1 byte, this is all we need.
}