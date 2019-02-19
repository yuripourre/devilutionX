/*
 * libsmackerdec - Smacker video decoder
 * Copyright (C) 2011 Barry Duncan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "../include/FileStream.h"
#include <stdlib.h>

namespace SmackerCommon {

static inline constexpr uint16_t B_SWAP16(uint16_t value)
{
    return
        ((value & 0xFF00u) >> 8u) |
        ((value & 0x00FFu) << 8u);
}

static inline constexpr uint32_t B_SWAP32(uint32_t value)
{
    return
        ((value & 0xFF000000u) >> 24u) |
        ((value & 0x00FF0000u) >>  8u) |
        ((value & 0x0000FF00u) <<  8u) |
        ((value & 0x000000FFu) << 24u);
}

static inline constexpr uint16_t B_PASS16(uint16_t const x) { return x; }
static inline constexpr uint32_t B_PASS32(uint32_t const x) { return x; }

#if B_LITTLE_ENDIAN == 1
# define B_LITTLE32(x) B_PASS32(x)
# define B_BIG32(x)    B_SWAP32(x)
# define B_LITTLE16(x) B_PASS16(x)
# define B_LITTLE16(x) B_PASS16(x)
# define B_BIG16(x) B_SWAP16(x)
#elif B_BIG_ENDIAN == 1
# define B_LITTLE32(x) B_SWAP32(x)
# define B_BIG32(x)    B_PASS32(x)
# define B_LITTLE16(x) B_SWAP16(x)
# define B_LITTLE16(x) B_SWAP16(x)
# define B_BIG16(x) B_PASS16(x)
#endif

bool FileStream::Open(const std::string &fileName)
{
	file.open(fileName.c_str(), std::ifstream::in | std::ifstream::binary);
	if (!file.is_open())
	{
		// log error
		return false;
	}

	return true;
}

bool FileStream::Is_Open()
{
	return file.is_open();
}

void FileStream::Close()
{
	file.close();
}

int32_t FileStream::ReadBytes(uint8_t *data, uint32_t nBytes)
{
	file.read(reinterpret_cast<char*>(data), nBytes);

	if (file.eof()) {
		return 0;
	}
	else if (file.fail()) {
		return 0;
	}
	else if (file.bad()) {
		return 0;
	}

	return static_cast<int32_t>(file.gcount());
}

uint32_t FileStream::ReadUint32LE()
{
	uint32_t value;
	file.read(reinterpret_cast<char*>(&value), 4);
	return value;
}

uint32_t FileStream::ReadUint32BE()
{
	uint32_t value;
	file.read(reinterpret_cast<char*>(&value), 4);
	return B_SWAP32(value);
}

uint16_t FileStream::ReadUint16LE()
{
	uint16_t value;
	file.read(reinterpret_cast<char*>(&value), 2);
	return value;
}

uint16_t FileStream::ReadUint16BE()
{
	uint16_t value;
	file.read(reinterpret_cast<char*>(&value), 2);
	return B_SWAP16(value);
}

uint8_t FileStream::ReadByte()
{
	uint8_t value;
	file.read(reinterpret_cast<char*>(&value), 1);
	return value;
}

bool FileStream::Seek(int32_t offset, SeekDirection direction)
{
	if (kSeekStart == direction) {
		file.seekg(offset, std::ios::beg);
	}
	else if (kSeekCurrent == direction) {
		file.seekg(offset, std::ios::cur);
	}

	// TODO - end seek

	if (file.bad())
	{
		// todo
		return false;
	}
	if (file.fail())
	{
		// todo
		return false;
	}

	return true;
}

bool FileStream::Skip(int32_t offset)
{
	return Seek(offset, kSeekCurrent);
}

bool FileStream::Is_Eos()
{
	return file.eof();
}

int32_t FileStream::GetPosition()
{
	return static_cast<int32_t>(file.tellg());
}

} // close namespace SmackerCommon
