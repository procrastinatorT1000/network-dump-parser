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

int main()
{
	string fileName = "some_shit";

	thread reader(fileReaderThread, &fileName);

	reader.join();


	return 0;
}
