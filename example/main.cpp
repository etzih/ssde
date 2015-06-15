/*
* This file is usage demo for SSDE (http://github.com/notnanocat/ssde).
* This file is not a subject to license, feel free to use it in any way
* You wish.
*/
#include "../ssde/ssde.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>


int main(int argc, const char *argv[])
{
	using namespace std;

	ios_base::sync_with_stdio(false);


	const string demo = /* bc to disassemble */
		"\x55"
		"\x31\xd2"
		"\x89\xe5"
		"\x8b\x45\x08"
		"\x56"
		"\x8b\x75\x0c"
		"\x53"
		"\x8d\x58\xff"
		"\x0f\xb6\x0c\x16"
		"\x88\x4c\x13\x01"
		"\x83\xc2\x01"
		"\x84\xc9"
		"\x75\xf1"
		"\x5b"
		"\x5e"
		"\x5d"
		"\xc3";


	/* ssde::ssde requires a (const) pointer to array of bytes (uint8_t) to be passed. */
	ssde_x86 dis(reinterpret_cast<const uint8_t *>(demo.c_str()));
	
	while (dis.next(), dis.ip < demo.length())
		/* call next() to iterate and make sure that IP doesn't overflow */
	{
		/* output address of the instruction */
		cout << setfill('0') << setw(8) << hex << dis.ip << ": ";
		
		for (unsigned int i = 0; i < dis.length; ++i)
			/* output instruction's bytes */
		{
			cout << setfill('0') << setw(2) << hex << (static_cast<unsigned int>(demo[dis.ip+i]) & 0xff);
		}

		if (dis.has_rel)
			/* if this instruction has relative address, print where it points to */
		{
			cout << " ; -> " << setfill('0') << setw(8) << hex << dis.abs;
		}

		cout << endl;
	}

	return 0;
}