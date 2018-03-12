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

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		cout<<"Specify dump file name as argument\n";
		return 0;
	}

	string fileName = argv[1];
//	Count writerArg = {.net1 = 10, .net2 = 10};

	cout<<"Parsing dump: "<<fileName<<"\n";

	thread reader(fileReaderThread, &fileName);
//	thread queueWriter(queueWriterThread, &writerArg);
	thread parser(dataParserThread);

	reader.join();
//	queueWriter.join();
	parser.join();


	return 0;
}
