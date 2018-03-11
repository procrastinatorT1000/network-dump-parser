//============================================================================
// Name        : main.cpp
// Author      : Slobodchikov RS
// Version     :
// Copyright   : romka rabotaet
// Description : Custom protocol parser
//============================================================================

#include <iostream>
#include <thread>
#include "netw_dump_parser.hpp"

using namespace std;

typedef struct{
	int net1;
	int net2;
}Count;

int main()
{
	string fileName = "some_shit";
	Count writerArg = {.net1 = 10, .net2 = 10};

	thread reader(fileReaderThread, &fileName);
	thread queueWriter(queueWriterThread, &writerArg);
	thread parser(dataParserThread, &fileName);

	reader.join();
	queueWriter.join();
	parser.join();


	return 0;
}
