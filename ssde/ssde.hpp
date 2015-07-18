/*
* The SSDE header file.
* Copyright (C) 2015, Constantine Shablya. See Copyright Notice in LICENSE.md
*/
#pragma once

#include <string>

#include <stdint.h>


/*
* The SSDE base class. This class is used to build disassembler
* classes and also provides polymorphism. Calling ssde::ssde
* constructors is prohibited. To create new ssde object, use
* SSDE derivation, specific to the architecture you need.
*
* Sample usage:
*   for (ssde_x86 dis(buffer); dis.dec(); dis.next())
*   {
*     ...
*   }
*/
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

	bool ip_overflow = false;               // IP is out of buffer's bounds. This field must be manually reset, otherwise ::next() won't do anything.

	size_t       ip;                        // Instruction pointer. Can be manually overriden.
	unsigned int length  = 0;               // Instruction length, in bytes. Can be manually overriden.

protected:
	const std::string &buffer;
};