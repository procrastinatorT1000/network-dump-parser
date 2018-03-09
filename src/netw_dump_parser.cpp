/*
 * netw_dump_parser.cpp
 *
 *  Created on: 5 марта 2018 г.
 *      Author: Slobodchikov RS
 */
//#include <fstream>
//#include <iostream>
//#include <stdint.h>
#include <queue>
#include "netw_dump_parser.hpp"

using namespace std;

queue<uint8_t> byteStream;
static const size_t portionBSize = 1000; /* every iteration reader trying to read this byte len */

/* object which reads data
 * from file, pushes it to std:queue */
DumpReader::DumpReader(string fileName)
{
	cout<<"constr\n";
	dump_file.open(fileName, ios_base::in | ios_base::binary);
	if(!dump_file)
	{
		cout<<"Can't open "<<fileName<<" file!\n";
	}
}

DumpReader::~DumpReader()
{
	cout<<"destr\n";
	if(dump_file)
	{
		dump_file.close();
	}
}

size_t DumpReader::read(uint8_t *bufPtr, size_t bufSize)
{
	cout<<"Read proc\n";

	size_t readBlen = 0;	/* byte length of data read from file with current iteration */

	if(dump_file)
	{
		readBlen = dump_file.readsome((char *)bufPtr, bufSize);
	}

	return readBlen;
}

void fileReaderThread(void *arg)
{
	string fileName = *(string *)arg;
	DumpReader dump(fileName);
	uint8_t *fileReadBuf = new uint8_t[portionBSize];
	size_t byteLen = 0;

	while(1)
	{
		byteLen = dump.read(fileReadBuf, portionBSize);
		if(byteLen == 0)
		{
			cout<<"Nothing to read\n";
			break;	/* nothing to read or file don't exists */
		}

		for(size_t i = 0; i < byteLen; i++)
		{
			byteStream.push(fileReadBuf[i]);	/* transmit data to parser thread */
		}
	}


	delete []fileReadBuf;
}

/* object which parses data from reader */

