/*
 * netw_dump_parser.cpp
 *
 *  Created on: 5 марта 2018 г.
 *      Author: Slobodchikov RS
 */
//#include <fstream>
//#include <iostream>
//#include <stdint.h>
#include "netw_dump_parser.hpp"

using namespace std;
/* object which reads data
 * from file, pushes it to std:queue */

DumpReader::DumpReader(char *fileName)
{
	cout<<"constr\n";
	dump_file.open(fileName, ios_base::in | ios_base::binary);
	if(!dump_file)
	{
		cout<<"Can't open "<<fileName<<" file!\n";
	}
	dataPtr = new uint8_t[portionBSize];
}

DumpReader::~DumpReader()
{
	cout<<"destr\n";
	if(dump_file)
	{
		dump_file.close();
	}
	if(dataPtr != NULL)
	{
		delete dataPtr;
	}
}

void DumpReader::read(void *)
{
	cout<<"Read proc\n";
}

/* object which parses data from reader */

