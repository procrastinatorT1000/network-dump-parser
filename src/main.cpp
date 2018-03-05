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

void t1()
{
	cout<<"t1\n";
	return;
}

int main() {

	DumpReader reader("hui");

	thread tt1(t1);

//	tt1.join();
//
//	reader.read();

	return 0;
}
