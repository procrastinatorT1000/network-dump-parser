/*
 * netw_dump_parser.hpp
 *
 *  Created on: 5 марта 2018 г.
 *      Author: Slobodchikov RS
 */

#ifndef NETW_DUMP_PARSER_HPP_
#define NETW_DUMP_PARSER_HPP_

#include <fstream>
#include <iostream>
#include <stdint.h>

using namespace std;

class DumpReader
{
	ifstream dump_file;

public:
	DumpReader(string fileName); /* open file and save file descriptor */
	~DumpReader();	/* close file and free memory */
	size_t read(uint8_t *bufPtr, size_t bufSize);
};

void fileReaderThread(void *arg);

#endif /* NETW_DUMP_PARSER_HPP_ */
