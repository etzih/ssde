/*
* The SSDE header file.
* Copyright (C) 2015, Constantine Shablya. See Copyright Notice in LICENSE.md
*/
#pragma once

#include <string>

#include <stdint.h>
#include <stddef.h>


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

	bool error_overflow = false;            // IP is out of buffer's bounds. This field must be manually reset, otherwise ::next() won't do anything.

	size_t       ip;                        // Instruction pointer. Can be manually overriden.
	unsigned int length  = 0;               // Instruction length, in bytes. Can be manually overriden.

protected:
	const std::string &buffer;
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

	bool dec() override final;

private:
	void reset_fields();

public:
	bool error_lock = false;                // LOCK prefix is not allowed.
	bool error_vex  = false;                // Instruction is only allowed to be VEX encoded.

	uint8_t group1 = 0;                     // Opcode prefix in 1st group, 0 if none. 1st group includes LOCK, REPNZ and REPZ prefixes.
	uint8_t group2 = 0;                     // Opcode prefix in 2nd group, 0 if none. 2nd group includes segment prefixes and/or branch hints.
	uint8_t group3 = 0;                     // Opcode prefix in 3rd group, 0 if none. 3rd group includes operand-size override prefix (p_66)
	uint8_t group4 = 0;                     // Opcode prefix in 4th group, 0 if none. 4th group includes address-size override prefix (p_67)

	bool    has_vex    = false;             // Has VEX prefix.
	uint8_t vex_size   = 0;                 // Size of VEX prefix (usually 2 or 3 bytes).
	uint8_t vex_reg    = 0;                 // VEX register specifier.
	bool    vex_r      = false;             // R field.
	bool    vex_x      = false;             // X field.
	bool    vex_b      = false;             // B field.
	bool    vex_w      = false;             // W field.
	bool    vex_l      = false;             // L field.

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

	bool         has_disp  = false;         // Has address displacement.
	unsigned int disp_size = 0;             // Size of address displacement, in bytes.
	uint32_t     disp      = 0;             // Displacement value.


	bool         has_imm   = false;         // Has immediate value.
	bool         has_imm2  = false;         // Has 2 immediate values.
	unsigned int imm_size  = 0;             // Size of the first immediate value, in bytes.
	unsigned int imm2_size = 0;             // Size of the second immediate value, in bytes.
	uint32_t     imm       = 0;             // First immediate value.
	uint32_t     imm2      = 0;             // Second immediate value.

	bool         has_rel  = false;          // Has relative address.
	unsigned int rel_size = 0;              // Size of relative address, in bytes.
	int32_t      rel      = 0;              // Relative address value.
	uint32_t     abs      = 0;              // Absolute address value.
};

/*
* SSDE disassembler for X86-64 architecture.
*/
class ssde_x8664 final : public ssde
{
public:
	using ssde::ssde;

	bool dec() override final;

private:
	void reset_fields();

public:
	bool error_lock = false;                // LOCK prefix is not allowed.

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

	bool         has_disp  = false;         // Has address displacement.
	unsigned int disp_size = 0;             // Size of address displacement, in bytes.
	uint32_t     disp      = 0;             // Displacement value.


	bool         has_imm   = false;         // Has immediate value.
	bool         has_imm2  = false;         // Has 2 immediate values.
	unsigned int imm_size  = 0;             // Size of the first immediate value, in bytes.
	unsigned int imm2_size = 0;             // Size of the second immediate value, in bytes.
	uint64_t     imm       = 0;             // First immediate value.
	uint64_t     imm2      = 0;             // Second immediate value.
	
	bool         has_rel  = false;          // Has relative address.
	unsigned int rel_size = 0;              // Size of relative address, in bytes.
	int32_t      rel      = 0;              // Relative address value.
	uint64_t     abs      = 0;              // Absolute address value.
};