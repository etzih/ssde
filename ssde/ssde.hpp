/*
* The SSDE header file.
* Copyright (C) 2015, Constantine Shablya. See Copyright Notice in LICENSE.txt
*/
#pragma once

#include <stdint.h>
#include <stddef.h>

/*
* The SSDE base class. This class is used to build disassembler
* classes and also provides polymorphism. Calling ssde::ssde
* constructors is prohibited. To create new ssde object, use
* SSDE derivation, specific to the architecture you need.
*
* Sample usage:
*   ssde_x86 dis(buffer, 0);
*   do {
*     dis.next();
*   } while (dis.ip < length_of_buffer);
*
* SSDE uses WEAK memory aliasing, so make sure data you pass
* it exists in the memory during the next() call. Make sure
* that SSDE won't overrun the buffer by checking that IP is
* < than instruction_buffer_length. Do it before each iteration.
*
* WEAK memory aliasing lets SSDE to be used to analyze large
* amounts of machine code while keeping great performance. 
* Heavily branched code can be easily analyzed due to cheap
* copy constructor.
*/
class ssde
{
public:
	ssde(const uint8_t *data, size_t pos = 0) :
		ip(pos),
		next_ip(pos),
		buffer(data)
	{
	}

	ssde(const ssde &from) :
		ip(from.ip),
		next_ip(from.next_ip),
		buffer(from.buffer)
	{
	}

	virtual void next() = 0;                // Advance to the next instruction.

	void inline set_ip(size_t pos = 0)      // Change IP.
	{
		ip      = pos;
		next_ip = pos;
	}

public:
	bool error         = false;             // Decoding error.
	bool error_opcode  = false;             // Bad opcode.
	bool error_operand = false;             // Bad operand(s).
	bool error_length  = false;             // Instruction is too long.

	size_t       ip;                        // Instruction pointer.
	unsigned int length  = 0;               // Instruction length, in bytes.

protected:
	size_t next_ip;

	const uint8_t *buffer;
};

/*
* SSDE disassembler for X86 architecture.
*/
class ssde_x86 final : public ssde
{
public:
	/*
	* Prefixes of X86.
	*/
	enum : uint8_t
	{
		p_none = 0,                         // No prefix.

		p_seg_cs = 0x2e,                    // CS segment prefix.
		p_seg_ss = 0x36,                    // SS segment prefix.
		p_seg_ds = 0x3e,                    // DS segment prefix.
		p_seg_es = 0x26,                    // ES segment prefix.
		p_seg_fs = 0x64,                    // FS segment prefix.
		p_seg_gs = 0x65,                    // GS segment prefix.
		p_lock   = 0xf0,                    // LOCK prefix.
		p_repnz  = 0xf2,                    // REPNZ prefix.
		p_repz   = 0xf3,                    // REPZ prefix.
		p_66     = 0x66,                    // Operand size override prefix.
		p_67     = 0x67,                    // Address size override prefix.

		p_branch_not_taken = 0x2e,          // Branch not taken hint.
		p_branch_taken     = 0x3e,          // Branch taken hint.
	};

	using ssde::ssde;

	void next() override final;

private:
	void reset_fields();

public:
	bool error_lock = false;                // LOCK prefix is not allowed.

	uint8_t group1 = 0;                     // Opcode prefix in 1st group, 0 if none. 1st group includes LOCK, REPNZ and REPZ prefixes.
	uint8_t group2 = 0;                     // Opcode prefix in 2nd group, 0 if none. 2nd group includes segment prefixes and/or branch hints.
	uint8_t group3 = 0;                     // Opcode prefix in 3rd group, 0 if none. 3rd group includes operand-size override prefix (p_66)
	uint8_t group4 = 0;                     // Opcode prefix in 4th group, 0 if none. 4th group includes address-size override prefix (p_67)

	uint8_t opcode1 = 0;                    // 1st opcode byte.
	uint8_t opcode2 = 0;                    // 2nd opcode byte.
	uint8_t opcode3 = 0;                    // 3rd opcode byte.

	bool    has_modrm = false;              // Has Mod R/M byte.
	uint8_t modrm_mod = 0;                  // Mod R/M address mode.
	uint8_t modrm_reg = 0;                  // Register number or opcode information.
	uint8_t modrm_rm  = 0;                  // Operand register.

	bool    has_sib   = false;              // Has SIB byte.
	uint8_t sib_scale = 0;                  // Index scale factor.
	uint8_t sib_index = 0;                  // Index register.
	uint8_t sib_base  = 0;                  // Base register.

	bool         has_imm   = false;         // Has immediate value.
	bool         has_imm2  = false;         // Has 2 immediate values.
	unsigned int imm_size  = 0;             // Size of the first immediate value, in bytes.
	unsigned int imm2_size = 0;             // Size of the second immediate value, in bytes.
	uint32_t     imm       = 0;             // First immediate value.
	uint32_t     imm2      = 0;             // Second immediate value.

	bool         has_disp  = false;         // Has address displacement.
	unsigned int disp_size = 0;             // Size of address displacement, in bytes.
	uint32_t     disp      = 0;             // Displacement value.
	
	bool         has_rel  = false;          // Has relative address.
	unsigned int rel_size = 0;              // Size of relative address, in bytes.
	int32_t      rel      = 0;              // Relative address value.
	uint32_t     abs      = 0;              // Absolute address value.
};