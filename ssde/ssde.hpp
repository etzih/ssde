/*
* The SSDE header file.
* Copyright (C) 2015, Constantine Shablya. See Copyright Notice in LICENSE.md
*/
#pragma once

#include <string>

#include <stdint.h>


class ssde
{
public:
	ssde(const std::string &data, size_t pos = 0) :
		ip(pos),
		buffer(data)
	{
	}

	ssde(const ssde &from) :
		ip(from.ip),
		buffer(from.buffer)
	{
	}


	virtual bool dec() = 0;                 // Decode instruction pointed by IP.

	void next()                             // Advance to the next instruction.
	{
		ip += length;
	}

public:
	bool error         = false;             // Decoding error.
	bool error_opcode  = false;             // Bad opcode.
	bool error_operand = false;             // Bad operand(s).
	bool error_length  = false;             // Instruction is too long.

	size_t       ip;                        // Instruction pointer. Can be manually overriden.
	unsigned int length  = 0;               // Instruction length, in bytes. Can be manually overriden.

protected:
	const std::string &buffer;
};