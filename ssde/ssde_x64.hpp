/*
* The SSDE header file for ssde_x64.cpp.
* Copyright (C) 2015, Constantine Shablya. See Copyright Notice in LICENSE.md
*/
#pragma once
#include "ssde.hpp"

/*
* SSDE disassembler for X86-64 architecture.
*/
class ssde_x64 final : public ssde
{
public:
	/*
	* Legacy X86 prefixes.
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

	void decode_prefixes();
	void decode_opcode();
	void decode_modrm();
	void decode_sib();
	void decode_imm();

	void vex_decode_pp(uint8_t pp);
	void vex_decode_mm(uint8_t mm);

public:
	bool error_lock = false;                // LOCK prefix is not allowed.
	bool error_novex = false;               // Instruction is only allowed to be VEX encoded.

	uint8_t group1 = 0;                     // Opcode prefix in 1st group, 0 if none. 1st group includes LOCK, REPNZ and REPZ prefixes.
	uint8_t group2 = 0;                     // Opcode prefix in 2nd group, 0 if none. 2nd group includes segment prefixes and/or branch hints.
	uint8_t group3 = 0;                     // Opcode prefix in 3rd group, 0 if none. 3rd group includes operand-size override prefix (p_66)
	uint8_t group4 = 0;                     // Opcode prefix in 4th group, 0 if none. 4th group includes address-size override prefix (p_67)

	union // W field.
	{
		bool rex_w = false;
		bool vex_w;
	};

	union // R field.
	{
		bool rex_r = false;
		bool vex_r;
	};

	union // X field.
	{
		bool rex_x = false;
		bool vex_x;
	};

	union // B field.
	{
		bool rex_b = false;
		bool vex_b;
	};

	bool    has_rex = false;                // Has REX prefix;

	bool    has_vex    = false;             // Has VEX prefix.
	bool    vex_zero   = false;             // Should zero or merge?; z field.
	uint8_t vex_size   = 0;                 // Size of VEX prefix (usually 2 or 3 bytes).
	uint8_t vex_reg    = 0;                 // VEX register specifier.
	uint8_t vex_opmask = 0;                 // VEX opmask register specifier.
	bool    vex_rr     = false;             // VEX R' field.
	bool    vex_sae    = false;             // VEX Broadcast/RC/SAE context.
	uint8_t vex_l      = 0;                 // VEX L field.

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

	bool    has_disp  = false;              // Has address displacement.
	int     disp_size = 0;                  // Size of address displacement, in bytes.
	int32_t disp      = 0;                  // Displacement value.

	bool     has_imm   = false;             // Has immediate value.
	bool     has_imm2  = false;             // Has 2 immediate values.
	int      imm_size  = 0;                 // Size of the first immediate value, in bytes.
	int      imm2_size = 0;                 // Size of the second immediate value, in bytes.
	uint64_t imm       = 0;                 // First immediate value.
	uint64_t imm2      = 0;                 // Second immediate value.

	bool     has_rel  = false;              // Has relative address.
	int      rel_size = 0;                  // Size of relative address, in bytes.
	int32_t  rel      = 0;                  // Relative address value.
	uint64_t abs      = 0;                  // Absolute address value.

private:
	uint16_t flags;
};
