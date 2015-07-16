/*
* The SSDE implementation for X86 instruction set.
* Copyright (C) 2015, Constantine Shablya. See Copyright Notice in LICENSE.md
*/
#include "ssde_x86.hpp"

#include <string>

#include <stdint.h>

/*
* Major amounts of information this code was based on ware taken from
* the "Intel(R) 64 and IA-32 Architectures Software Developer's Manual".
* If You are unfamiliar with the X86 architecture, it's recommended that
* you read the manual first. The manuals can be obtained @
*   http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html
*
* Basic architecture @
*   http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-1-manual.pdf
* Instruction set reference @
*   http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.pdf
*/

enum : uint16_t
{
	none = 0,

	rm  = 1 << 0, // expect Mod byte
	ex  = 1 << 1, // expect Mod opcode extension
	rel = 1 << 2, // instruction's imm is a relative address
	i8  = 1 << 3, // has 8 bit imm 
	i16 = 1 << 4, // has 16 bit imm
	i32 = 1 << 5, // has 32 bit imm, which can be turned to 16 with 66 prefix
	am  = 1 << 6, // instruction uses address mode, imm is a memory address
	vx  = 1 << 7, // instruction requires a VEX prefix
	mp  = 1 << 8, // instruction has a mandatory 66 prefix

	r8  = i8  | rel,
	r32 = i32 | rel,

	error = (uint16_t)-1
};

/* 1st opcode flag table */
static const uint16_t op_table[256] =
{
	/*x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7  |  x8  |  x9  |  xA  |  xB  |  xC  |  xD  |  xE  |  xF */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , none ,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , error, /* 0x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , none ,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , none , /* 1x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none ,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none , /* 2x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none ,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none , /* 3x */
	 none , none , none , none , none , none , none , none , none , none , none , none , none , none , none , none , /* 4x */
	 none , none , none , none , none , none , none , none , none , none , none , none , none , none , none , none , /* 5x */
	 none , none ,  rm  ,  rm  , error, error, error, error,  i32 ,rm|i32,  i8  , rm|i8, none , none , none , none , /* 6x */
	  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  , /* 7x */
	 rm|i8,rm|i32, rm|i8, rm|i8,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 8x */
	 none , none , none , none , none , none , none , none , none , none,i32|i16, none , none , none , none , none , /* 9x */
	i32|am,i32|am,i32|am,i32|am, none , none , none , none ,  i8  ,  i32 , none , none , none , none , none , none , /* Ax */
	  i8  ,  i8  ,  i8  ,  i8  ,  i8  ,  i8  ,  i8  ,  i8  ,  i32 ,  i32 ,  i32 ,  i32 ,  i32 ,  i32 ,  i32 ,  i32 , /* Bx */
	 rm|i8, rm|i8,  i16 , none ,  rm  ,  rm  , rm|i8,rm|i32,i16|i8, none ,  i16 , none , none ,  i8  , none , none , /* Cx */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i8  , none , none ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* Dx */
	  r8  ,  r8  ,  r8  ,  r8  ,  i8  ,  i8  ,  i8  ,  i8  ,  r32 ,  r32,i32|i16,  r8  , none , none , none , none , /* Ex */
	 error, none , error, error, none , none , error, error, none , none , none , none , none , none ,  rm  ,  rm  , /* Fx */
};

/*
* 2nd opcode flag table
* 0F xx
*/
static const uint16_t op_table_0f[256] =
{
	/*x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7  |  x8  |  x9  |  xA  |  xB  |  xC  |  xD  |  xE  |  xF */
	  rm  ,  rm  ,  rm  ,  rm  , error, error, none , error, none , none , error, none , error,  rm  , none , error, /* 0x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 1x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  , error,  rm  , error,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 2x */
	 none , none , none , none , none , none , error, none , error, error, error, error, error, error, error, error, /* 3x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 4x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 5x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 6x */
	 rm|i8, rm|i8, rm|i8, rm|i8,  rm  ,  rm  ,  rm  , none ,  rm  ,  rm  , error, error,  rm  ,  rm  ,  rm  ,  rm  , /* 7x */
	  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 , /* 8x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 9x */
	 none , none , none ,  rm  , rm|i8,  rm  , error, error, none , none , none ,  rm  , rm|i8,  rm  ,  rm  ,  rm  , /* Ax */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , none ,  i8  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* Bx */
	  rm  ,  rm  , rm|i8,  rm  , rm|i8, rm|i8, rm|i8,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* Cx */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* Dx */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* Ex */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* Fx */
};

/*
* 3rd opcode flag table
* 0F 38 xx
*/
static const uint16_t op_table_38[256] =
{
	/*x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7  |  x8  |  x9  |  xA  |  xB  |  xC  |  xD  |  xE  |  xF */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , vx|rm, vx|rm, error, error, /* 0x */
	 mp|rm, error, error, error, mp|rm, mp|rm, error, mp|rm, vx|rm, error, vx|rm, error,  rm  ,  rm  ,  rm  , error, /* 1x */
	 mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, error, error, mp|rm, mp|rm, mp|rm, mp|rm, vx|rm, vx|rm, error, error, /* 2x */
	 mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, error, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, /* 3x */
	 mp|rm, mp|rm, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 4x */
	 error, error, error, error, error, error, error, error, vx|rm, vx|rm, error, error, error, error, error, error, /* 5x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 6x */
	 error, error, error, error, error, error, error, error, vx|rm, vx|rm, error, error, error, error, error, error, /* 7x */
	 mp|rm, mp|rm, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 8x */
	 error, error, error, error, error, error, vx|rm, vx|rm, vx|rm, error, vx|rm, error, vx|rm, error, vx|rm, error, /* 9x */
	 error, error, error, error, error, error, vx|rm, vx|rm, vx|rm, error, vx|rm, error, vx|rm, error, vx|rm, error, /* Ax */
	 error, error, error, error, error, error, vx|rm, vx|rm, vx|rm, error, vx|rm, error, vx|rm, error, vx|rm, error, /* Bx */
	 error, error, error, error, error, error, error, error,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , error, error, /* Cx */
	 error, error, error, error, error, error, error, error, error, error, error,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* Dx */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* Ex */
	  rm  ,  rm  , error, error, error, error,  rm  , error, error, error, error, error, error, error, error, error, /* Fx */
};

/*
* 3rd opcode flag table
* 0F 3A xx
*/
static const uint16_t op_table_3a[256] =
{
	/* x0   |   x1   |   x2   |   x3   |   x4   |   x5   |   x6   |   x7   |   x8   |   x9   |   xA   |   xB   |   xC   |   xD   |   xE   |   xF  */
	  error ,  error ,  error ,  error ,  error ,  error ,vx|rm|i8,  error ,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,   rm   , /* 0x */
	  error ,  error ,  error ,  error ,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,vx|rm|i8,vx|rm|i8,  error ,  error ,  error ,  error ,  error ,  error , /* 1x */
	mp|rm|i8,mp|rm|i8,mp|rm|i8,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 2x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 3x */
	  mp|rm ,  mp|rm ,mp|rm|i8,  error ,  error ,  error ,  error ,  error ,  error ,  error ,vx|rm|i8,vx|rm|i8,vx|rm|i8,  error ,  error ,  error , /* 4x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 5x */
	mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,  error ,  error ,  error ,  error ,vx|rm|i8,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 6x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 7x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 8x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 9x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* Ax */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* Bx */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,mp|rm|i8,  error ,  error ,  error , /* Cx */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* Dx */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* Ex */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* Fx */
};


void ssde_x86::reset_fields()
{
	length = 0;

	error         = false;
	error_opcode  = false;
	error_operand = false;
	error_length  = false;
	error_lock    = false;
	error_novex   = false;

	has_modrm = false;
	has_sib   = false;
	has_imm   = false;
	has_imm2  = false;
	has_disp  = false;
	has_rel   = false;
	has_vex   = false;

	group1 = 0;
	group2 = 0;
	group3 = 0;
	group4 = 0;

	opcode1 = 0;
	opcode2 = 0;
	opcode3 = 0;

	has_vex = false;
	vex_reg = 0;
	vex_r   = false;
	vex_x   = false;
	vex_b   = false;
	vex_w   = false;
	vex_l   = 0;
}

bool ssde_x86::dec()
{
	if (ip_overflow)
		return false;

	if (ip >= buffer.length())
	{
		if (ip > buffer.length())
			ip_overflow = true;

		return false;
	}


	reset_fields();

	for (unsigned int x = 0; x < 15; ++x, ++length)
		/*
		* This is prefix analyzer. It behaves exactly the
		* same way real CPUs analyze instructions for
		* prefixes. Normally, each instruction is allowed
		* to have up to 4 prefixes from each group. Though,
		* in cases when instruction has more prefixes, it
		* will ignore any prefix it meets if there was a
		* prefix from the same group before it. Instruction
		* decoders can only handle words up to 15 bytes long,
		* if the word is longer than that, decoder will fail.
		*/
	{
		uint8_t prefix = buffer[ip + length];

		/* 1st group */
		if (prefix == p_lock  ||
		    prefix == p_repnz ||
		    prefix == p_repz)
		{
			if (group1 == p_none)
				group1 = prefix;

			continue;
		}

		/* 2nd group */
		if (prefix == p_seg_cs || prefix == p_seg_ss ||
		    prefix == p_seg_ds || prefix == p_seg_es ||
		    prefix == p_seg_fs || prefix == p_seg_gs
			/* p_branch_not_taken, p_branch_taken, */)
		{
			if (group2 == p_none)
				group2 = prefix;

			continue;
		}

		/* 3rd group */
		if (prefix == p_66)
		{
			if (group3 == p_none)
				group3 = prefix;

			continue;
		}

		/* 4th group */
		if (prefix == p_67)
		{
			if (group4 == p_none)
				group4 = prefix;

			continue;
		}

		break;
	}

	
	uint16_t flags = ::error;

	if ((static_cast<uint8_t>(buffer[ip + length]) == 0xc4 ||
	     static_cast<uint8_t>(buffer[ip + length]) == 0xc5 ||
	     static_cast<uint8_t>(buffer[ip + length]) == 0x62) &&
	    buffer[ip + length+1] & 0x80)
		/* looks like we've found a VEX prefix */
	{
		has_vex = true;

		if (group1 != 0 ||
		    group2 != 0 ||
		    group3 != 0 ||
		    group4 != 0)
			/* VEX-encoded instructions are not allowed to be preceeded by legacy prefixes */
		{
			error = true;
			error_opcode = true;
		}


		uint8_t prefix = buffer[ip + length++];

		if (prefix == 0x62)
			/* this is a 4 byte VEX */
		{
			vex_size = 4;

			// TODO(notnanocat): implement
		}
		else
		{
			if (prefix == 0xc4)
				/* this is a 3 byte VEX */
			{
				vex_size = 3;


				uint8_t vex_1 = buffer[ip + length++];

				vex_r = vex_1 & 0x80 ? true : false;
				vex_x = vex_1 & 0x40 ? true : false;
				vex_b = vex_1 & 0x20 ? true : false;

				switch (vex_1 & 0x1f)
					/* decode first one or two opcode bytes */
				{
				case 0x01:
					{
						opcode1 = 0x0f;
					}
					break;

				case 0x02:
					{
						opcode1 = 0x0f;
						opcode2 = 0x38;
					}
					break;

				case 0x03:
					{
						opcode1 = 0x0f;
						opcode2 = 0x3a;
					}
					break;

				default:
					{
						error = true;
						error_opcode = true;
						error_novex = true;
					}
					break;
				}
			}
			else
			{
				vex_size = 2;

				opcode1 = 0x0f;
			}


			uint8_t vex_2 = buffer[ip + length++];

			if (prefix == 0xc4)
			{
				vex_w = vex_2 & 0x80 ? true : false;
			}
			else
			{
				vex_r = vex_2 & 0x80 ? true : false;
			}

			vex_l = vex_2 & 0x04 ? 1 : 0;
			vex_reg = ~vex_2 & 0x78 >> 3;

			switch (vex_2 & 0x02)
				/* decode prefix */
			{
			case 0x01:
				{
					group3 = p_66;
				}
				break;

			case 0x02:
				{
					group1 = p_repz;
				}
				break;

			case 0x03:
				{
					group1 = p_repnz;
				}
				break;

			default:
				break;
			}
		}

		if (opcode1 == 0x0f)
		{
			if (opcode2 == 0x38)
			{
				opcode3 = buffer[ip + length++];
				flags   = op_table_38[opcode3];
			}
			else if (opcode2 == 0x3a)
			{
				opcode3 = buffer[ip + length++];
				flags   = op_table_3a[opcode3];
			}
			else
			{
				opcode2 = buffer[ip + length++];
				flags   = op_table_0f[opcode2];
			}
		}
	}
	else
		/* instruction operands are written normal way */
	{
		opcode1 = buffer[ip + length++];

		if (opcode1 == 0x0f)
		{
			opcode2 = buffer[ip + length++];

			if (opcode2 == 0x38)
			{
				opcode3 = buffer[ip + length++];
				flags   = op_table_38[opcode3];
			}
			else if (opcode2 == 0x3a)
			{
				opcode3 = buffer[ip + length++];
				flags   = op_table_3a[opcode3];
			}
			else
			{
				flags = op_table_0f[opcode2];
			}
		}
		else if (opcode1 == 0xf6 || opcode1 == 0xf7)
			/*
			* These are two exceptional opcodes that extend
			* using 3 bits of Mod R/M byte and they lack
			* consistent flags. Instead of creating a new
			* flags table for each extended opcode, I decided
			* to put this little bit of code that is dedicated
			* to these two exceptional opcodes.
			*/
		{
			switch (buffer[ip + length] >> 3 & 0x07)
			{
			case 0x00:
			case 0x01:
				{
					if (opcode1 == 0xf6)
						flags = rm | i8;

					if (opcode1 == 0xf7)
						flags = rm | i32;
				}
				break;

			default:
				{
					flags = rm;
				}
				break;
			}
		}
		else
			/* this is a regular single opcode instruction */
		{
			flags = op_table[opcode1];
		}

		if (flags & ::vx)
			/* this instruction can only be VEX-encoded */
		{
			error = true;
			error_novex = true;
		}
	}

	if (flags != ::error)
		/* it's not a bullshit instruction */
	{
		if (flags & ::mp && group3 != p_66)
			/* this instruction lacks mandatory 66 prefix */
		{
			error = true;
			error_opcode = true;
		}

		if (flags & ::rm)
			/* this instruction has a Mod byte, decode it */
		{
			uint8_t modrm_byte = buffer[ip + length++];

			has_modrm = true;
			modrm_mod = modrm_byte >> 6 & 0x03;
			modrm_reg = modrm_byte >> 3 & 0x07;
			modrm_rm  = modrm_byte      & 0x07;

			switch (modrm_mod)
			{
			case 0x00:
				if (group4 == p_67)
				{
					if (modrm_rm == 0x06)
					{
						has_disp  = true;
						disp_size = 2;
					}
				}
				else
				{
					if (modrm_rm == 0x04)
						has_sib = true;

					if (modrm_rm == 0x05)
					{
						has_disp  = true;
						disp_size = 4;
					}
				}
				break;

			case 0x01:
				{
					if (group4 != p_67 && modrm_rm == 0x04)
						has_sib = true;

					has_disp  = true;
					disp_size = 1;
				}
				break;

			case 0x02:
				{
					if (group4 != p_67 && modrm_rm == 0x04)
						has_sib = true;

					has_disp  = true;
					disp_size = group4 == p_67 ? 2 : 4;
				}
				break;

			case 0x03:
				if (group1 == p_lock)
					/* LOCK prefix is not allowed to be used with Mod R */
				{
					error = true;
					error_lock = true;
				}
				break;

			default:
				break;
			}

			if (has_sib)
				/* if instruction has a SIB byte, decode it, too */
			{
				uint8_t sib_byte = buffer[ip + length++];

				sib_scale = 1 << (sib_byte >> 6 & 0x03);
				sib_index = sib_byte >> 3 & 0x07;
				sib_base  = sib_byte      & 0x07;

				if (sib_index == 0x04)
					/* index register can't be ESP */
				{
					error = true;
					error_opcode = true;
				}
			}

			if (has_disp)
			{
				disp = 0;

				for (unsigned int i = 0; i < disp_size; i++)
					disp |= buffer[ip + length++] << i*8;
			}
		}
		else if (group1 == p_lock)
			/* LOCK prefix only makes sense for Mod M */
		{
			error = true;
			error_lock = true;
		}


		if (flags & ::am)
			/* address mode instructions behave a little differently */
		{
			has_imm  = true;
			imm_size = group4 == p_67 ? 2 : 4;
		}
		else
		{
			if (flags & ::i32)
			{
				has_imm  = true;
				imm_size = group3 == p_66 ? 2 : 4;
			}

			if (flags & ::i16)
			{
				if (has_imm)
				{
					has_imm2  = true;
					imm2_size = 2;
				}
				else
				{
					has_imm  = true;
					imm_size = 2;
				}
			}

			if (flags & ::i8)
			{
				if (has_imm)
				{
					has_imm2  = true;
					imm2_size = 1;
				}
				else
				{
					has_imm  = true;
					imm_size = 1;
				}
			}
		}

		if (has_imm)
		{
			imm = 0;

			for (unsigned int i = 0; i < imm_size; ++i)
				imm |= buffer[ip + length++] << i*8;


			if (has_imm2)
			{
				imm2 = 0;

				for (unsigned int i = 0; i < imm2_size; ++i)
					imm2 |= buffer[ip + length++] << i*8;
			}
		}

		if (flags & ::rel)
			/* this instruction has relative address, move imm to rel */
		{
			has_imm = false;

			rel_size = imm_size;
			rel = imm;

			if (rel & (1 << (rel_size*8 - 1)))
				/* rel is signed, extend the sign if needed */
			{
				switch (rel_size)
				{
				case 1:
					{
						rel |= 0xffffff00;
					}
					break;

				case 2:
					{
						rel |= 0xffff0000;
					}
					break;

				default:
					break;
				}
			}

			abs = ip + length + rel;

			has_rel = true;
		}


		if (length > 15)
			/* CPU can't handle instructions longer than 15 bytes */
		{
			length = 15;

			error = true;
			error_length = true;
		}
	}
	else
	{
		error = true;
		error_opcode = true;

		length = 1;
	}

	return true;
}