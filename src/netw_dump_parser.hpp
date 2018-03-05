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
	uint8_t *dataPtr;
	static const size_t portionBSize = 1000; /* every iteration reader trying to read this byte len */
	size_t readBlen = 0;	/* byte length of data read from file with current iteration */

public:
	DumpReader(char *fileName); /* open file and save file descriptor */
	~DumpReader();	/* close file and free memory */
	void read(void *);
};


#endif /* NETW_DUMP_PARSER_HPP_ */
