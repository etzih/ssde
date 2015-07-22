**The Small Scalable Disassembler Engine**

Copyright (C) 2015, Constantine Shablya. See Copyright Notice in LICENSE.md

SSDE is a small, scalable disassembly stream engine, purposed to analyze
machine code and retrieve information on instructions (their length, opcode,
correctness, etc).

Check *doc/manual_en.txt* or *doc/manual_ru.txt* for information about SSDE
and documentation in the language you speak.

Check *example/* to see how SSDE can be used.

         Supported architectures and extensions
	 ______________________________________________
	|     |                                        |
	| x86 | VMX, AES, SHA, MMX                     |
	|  /  | SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2 |
	| x64 | AVX, AVX2, FMA3                        |
	|_____|________________________________________|

           List of machines SSDE was tested on
	 ______________________________________________
	|                   |                  |       |
	| Core i3 2350m     | Windows 8.1 x64  | 64 LE |
	| ARMv6 processor   | Raspbian 7       | 32 LE |
	|___________________|__________________|_______|
