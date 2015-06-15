/*
* The SSDE implementation for X86 instruction set.
* Copyright (C) 2015, Constantine Shablya. See Copyright Notice in LICENSE.txt
*/
#include "ssde.hpp"

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

enum : uint8_t
{
	none = 0,

	rm  = 1 << 0,
	ex  = 1 << 1,
	rel = 1 << 2,
	i8  = 1 << 3,
	i16 = 1 << 4,
	i32 = 1 << 5,
	am  = 1 << 6,

	r8  = i8  | rel,
	r32 = i32 | rel,

	error = (uint8_t)-1
};

/* 1st opcode flag table */
static const uint8_t op_table[256] =
{
	/*x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7  |  x8  |  x9  |  xA  |  xB  |  xC  |  xD  |  xE  |  xF */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , none ,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , error, /* 0x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , none ,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , none , /* 1x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none ,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none , /* 2x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none ,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none , /* 3x */
	 none , none , none , none , none , none , none , none , none , none , none , none , none , none , none , none , /* 4x */
	 none , none , none , none , none , none , none , none , none , none , none , none , none , none , none , none , /* 5x */
	 none , none ,  i8  ,  i8  , error, error, error, error,  i32 ,rm|i32,  i8  , rm|i8, none , none , none , none , /* 6x */
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
static const uint8_t op_table_0f[256] =
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
static const uint8_t op_table_38[256] =
{
	/*x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7  |  x8  |  x9  |  xA  |  xB  |  xC  |  xD  |  xE  |  xF */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , error, error, error, error, /* 0x */
	  rm  , error, error, error,  rm  ,  rm  , error,  rm  , error, error, error, error,  rm  ,  rm  ,  rm  , error, /* 1x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , error, error,  rm  ,  rm  ,  rm  ,  rm  , error, error, error, error, /* 2x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , error,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 3x */
	  rm  ,  rm  , error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 4x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 5x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 6x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 7x */
	  rm  ,  rm  , error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 8x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 9x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* Ax */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* Bx */
	 error, error, error, error, error, error, error, error,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , error, error, /* Cx */
	 error, error, error, error, error, error, error, error, error, error, error,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* Dx */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* Ex */
	  rm  ,  rm  , error, error, error, error,  rm  , error, error, error, error, error, error, error, error, error, /* Fx */
};

/*
* 3rd opcode flag table
* 0F 3A xx
*/
static const uint8_t op_table_3a[256] =
{
	/*x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7  |  x8  |  x9  |  xA  |  xB  |  xC  |  xD  |  xE  |  xF */
	 error, error, error, error, error, error, error, error, rm|i8, rm|i8, rm|i8, rm|i8, rm|i8, rm|i8, rm|i8,  rm  , /* 0x */
	 error, error, error, error, rm|i8, rm|i8, rm|i8, rm|i8, error, error, error, error, error, error, error, error, /* 1x */
	 rm|i8, rm|i8, rm|i8, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 2x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 3x */
	  rm  ,  rm  , rm|i8, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 4x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 5x */
	 rm|i8, rm|i8, rm|i8, rm|i8, error, error, error, error, error, error, error, error, error, error, error, error, /* 6x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 7x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 8x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* 9x */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* Ax */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* Bx */
	 error, error, error, error, error, error, error, error, error, error, error, error, rm|i8, error, error, error, /* Cx */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* Dx */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* Ex */
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, /* Fx */
};

void ssde_x86::reset_fields()
{
	length = 0;

	error         = false;
	error_opcode  = false;
	error_operand = false;
	error_length  = false;
	error_lock    = false;

	has_modrm = false;
	has_sib   = false;
	has_imm   = false;
	has_imm2  = false;
	has_disp  = false;
	has_rel   = false;

	group1 = 0;
	group2 = 0;
	group3 = 0;
	group4 = 0;

	opcode1 = 0;
	opcode2 = 0;
	opcode3 = 0;
}

void ssde_x86::next()
{
	reset_fields();

	ip = next_ip;

	for (unsigned int x = 0; x < 15; ++x, ++next_ip)
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
		uint8_t prefix = buffer[next_ip];

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

	uint8_t flags = ::error;


	opcode1 = buffer[next_ip++];

	if (opcode1 == 0x0f)
		/* 2 and 3 byte opcodes' 1st byte is 0F */
	{
		opcode2 = buffer[next_ip++];

		if (opcode2 == 0x0f)
			/*
			* SSDE only provides basic support for 3DNow!
			* due to it being very unpopular since 1998
			* and even AMD decided to drop support for it
			* in 2010.
			*
			* All 3DNow! 0F 0F xx instructions have exactly
			* same length and none of them have any immediates.
			* Instruction encoding here is very fucked here,
			* regular instructions have opcode byte going
			* first and Mod R/M byte going after. But in
			* 3DNow! AMD decided that it would be a great
			* idea to have Mod byte go first and the opcode
			* after it. We solve this issue by treating
			* third opcode like a 8 bit immediate and then
			* moving it to opcode3 field.
			*/
		{
			flags = ::rm | ::i8;
		}
		else if (opcode2 == 0x38)
			/* a 3 byte opcode's 2nd byte is either 38 */
		{
			opcode3 = buffer[next_ip++];
			flags   = op_table_38[opcode3];
		}
		else if (opcode2 == 0x3a)
			/* or 3A */
		{
			opcode3 = buffer[next_ip++];
			flags   = op_table_3a[opcode3];
		}
		else
		{
			flags = op_table_0f[opcode2];
		}
	}
	else
	{
		flags = op_table[opcode1];

		if (opcode1 == 0xf6 || opcode1 == 0xf7)
			/*
			* These are two exceptional opcodes that extend
			* using 3 bits of Mod R/M byte and they lack
			* consistent flags. Instead of creating a new
			* flags table for each extended opcode, I decided
			* to put this little bit of code that is dedicated
			* to these two exceptional opcodes.
			*/
		{
			switch (buffer[ip] >> 3 & 0b111)
			{
			case 0b000:
			case 0b001:
				{
					if (opcode1 == 0xf6)
						flags = rm | i8;

					if (opcode1 == 0xf7)
						flags = rm | i32;
				}
				break;

			case 0b010:
			case 0b011:
			case 0b100:
			case 0b101:
			case 0b110:
			case 0b111:
				{
					flags = rm;
				}
				break;

			default:
				break;
			}
		}
	}

	if (flags != ::error)
		/* it's not a bullshit instruction */
	{
		if (flags & ::rm)
			/* this instruction has a Mod R/M byte, decode it */
		{
			uint8_t modrm_byte = buffer[next_ip++];

			has_modrm = true;
			modrm_mod = modrm_byte >> 6 & 0b11;
			modrm_reg = modrm_byte >> 3 & 0b111;
			modrm_rm  = modrm_byte      & 0b111;

			switch (modrm_mod)
			{
			case 0b00:
				if (group4 == p_67)
				{
					if (modrm_rm == 0b110)
					{
						has_disp  = true;
						disp_size = 2;
					}
				}
				else
				{
					if (modrm_rm == 0b100)
						has_sib = true;

					if (modrm_rm == 0b101)
					{
						has_disp  = true;
						disp_size = 4;
					}
				}
				break;

			case 0b01:
				{
					if (group4 != p_67 && modrm_rm == 0b100)
						has_sib = true;

					has_disp  = true;
					disp_size = 1;
				}
				break;

			case 0b10:
				{
					if (group4 != p_67 && modrm_rm == 0b100)
						has_sib = true;

					has_disp  = true;
					disp_size = group4 == p_67 ? 2 : 4;
				}
				break;

			case 0b11:
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
				uint8_t sib_byte = buffer[next_ip++];

				sib_scale = 1 << (sib_byte >> 6 & 0b11);
				sib_index = sib_byte >> 3 & 0b111;
				sib_base  = sib_byte      & 0b111;

				if (sib_index == 0b100)
					/* index register can't be (E/R)SP */
				{
					error = true;
					error_opcode = true;
				}
			}

			if (has_disp)
			{
				disp = 0;

				for (unsigned int i = 0; i < disp_size; i++)
					disp |= buffer[next_ip++] << i*8;
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
				imm |= buffer[next_ip++] << i*8;


			if (has_imm2)
			{
				imm2 = 0;

				for (unsigned int i = 0; i < imm2_size; ++i)
					imm2 |= buffer[next_ip++] << i*8;
			}
		}

		if (opcode2 == 0x0f)
			/* this is 3DNow! opcode, move imm to opcode3 */
		{
			has_imm = false;
			opcode3 = imm;
		}

		if (flags & ::rel)
			/* this instruction has relative address, move imm to rel */
		{
			has_imm = false;

			rel_size = imm_size;
			rel = imm;

			if (rel & (rel_size*8 - 1))
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


			abs = next_ip + rel;

			has_rel = true;
		}


		length = next_ip - ip;

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
}