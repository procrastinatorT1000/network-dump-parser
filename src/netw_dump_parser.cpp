/*
 * netw_dump_parser.cpp
 *
 *  Created on: 5 марта 2018 г.
 *      Author: Slobodchikov RS
 */
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <queue>
#include <memory.h>
#include <cstdint>
#include <arpa/inet.h>
#include "netw_dump_parser.hpp"

using namespace std;

queue<uint8_t> byteStream;	/** transport data from reader thread to parser thread */
static const size_t portionBSize = 1000; /* every iteration reader trying to read this byte len */

/* object which reads data
 * from file, pushes it to std:queue */
class DumpReader
{
	ifstream dump_file;

public:
	DumpReader(string fileName); /* open file and save file descriptor */
	~DumpReader();	/* close file and free memory */
	size_t read(uint8_t *bufPtr, size_t bufSize);
};

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

#pragma pack(push, 1)

typedef struct
{
	uint8_t src[4];
	uint8_t dst[4];
}IP_Net_1_t;

typedef struct
{
	uint32_t src[6];
	uint32_t dst[6];
}IP_Net_2_t;

typedef struct
{
	uint8_t protocol;
	uint16_t dataSize;
	uint16_t headCRC;
	/* next bytes is data */
}Proto_Info;

typedef struct
{
	uint8_t netVer;
	IP_Net_1_t IPnet1;
	Proto_Info protoInfo;
}Net1_Header;

typedef struct
{
	uint8_t netVer;
	IP_Net_1_t IPnet1;
	Proto_Info protoInfo;
}Net2_Header;

#pragma pack(pop)


class Parser_Net1
{
private:
	uint8_t protoType;	/* transport protocol type V1 V2 */
	uint16_t dataBSize;	/* transport data protocol byte size */
	uint32_t packCount;	/* NetworkV1 packet counter */
	uint32_t uniqIPcount;	/* unique IP counter */


public:
	Parser_Net1()
	{
		protoType = 255;
		dataBSize = 0;
		packCount = 0;
		uniqIPcount = 0;
	};
//	~Parser_Net1(){};

	int parseHeader(const uint8_t *head, const uint16_t headSize)
	{
		if(headSize != sizeof(Net1_Header))
		{
			cout<<"Net1Parser Incorrect header size\n";
			return false;
		}

		Net1_Header headerBigEnd;
		memcpy(&headerBigEnd, head, headSize);

		protoType = headerBigEnd.protoInfo.protocol;
		dataBSize = ntohs(headerBigEnd.protoInfo.dataSize);
		packCount++;
		uniqIPcount++;	/* TODO: it's workaround, delete */

		return true;
	};
	uint8_t getLastProtoType()
	{
		return protoType;
	};
	uint16_t getLastDataBSize()
	{
		return dataBSize;
	};
	uint16_t getPackCount()
	{
		return packCount;
	};
	uint16_t getUniqueIPcount()
	{
		return uniqIPcount;
	};
};

class Parser_Net2
{
private:
	uint8_t protoType;	/* transport protocol type V1 V2 */
	uint16_t dataBSize;	/* transport data protocol byte size */
	uint32_t packCount;	/* NetworkV1 packet counter */
	uint32_t uniqIPcount;	/* unique IP counter */


public:
	Parser_Net2()
	{
		protoType = 255;
		dataBSize = 0;
		packCount = 0;
		uniqIPcount = 0;
	};
//	~Parser_Net2()
//	{};

	int parseHeader(const uint8_t *head, const uint16_t headSize)
	{
		if(headSize != sizeof(Net2_Header))
		{
			cout<<"Net2Parser Incorrect header size\n";
			return false;
		}

		Net2_Header headerBigEnd;
		memcpy(&headerBigEnd, head, headSize);

		protoType = headerBigEnd.protoInfo.protocol;
		dataBSize = ntohs(headerBigEnd.protoInfo.dataSize);
		packCount++;
		uniqIPcount++;	/* TODO: it's workaround, delete */

		return true;
	};
	uint8_t getLastProtoType()
	{
		return protoType;
	};
	uint16_t getLastDataBSize()
	{
		return dataBSize;
	};
	uint16_t getPackCount()
	{
		return packCount;
	};
	uint16_t getUniqueIPcount()
	{
		return uniqIPcount;
	};
};


/* returns true if data was read
 * false if queue don't contain byteNum elements */
int readDataFromQueue(uint8_t *buf, size_t byteNum)
{
	if(byteStream.size()>= byteNum)
	{
		for(size_t i = 0; i < byteNum; i++)
		{
			buf[i] = byteStream.front(); /* get byte */
			byteStream.pop(); /* delete byte from queue */
		}

		return true;
	}
	else
	{
		return false;
	}
}


void queueWriterThread(void *arg)
{
	uint8_t *byte;
	Net1_Header net1;

	typedef struct{
		int net1;
		int net2;
	}Count;

	Count count = *(Count *)arg;

	net1.netVer = 1;
	net1.IPnet1 = {0x12, 0x34, 0x56, 0x78};
	net1.protoInfo.dataSize = 1;
	net1.protoInfo.headCRC = 0x5555;
	net1.protoInfo.protocol = 1;

	Net2_Header net2;

	net2.netVer = 2;
	net2.IPnet1 = {0x12, 0x34, 0x56, 0x78, 0x90, 0x12};
	net2.protoInfo.dataSize = 1;
	net2.protoInfo.headCRC = 0x5555;
	net2.protoInfo.protocol = 1;

	byte = (uint8_t*)&net1;

	for(int j = 0; j < count.net1; j++)
	{
		for(size_t i  = 0; i < sizeof(net1); i++)
		{
			byteStream.push(byte[i]);
		}
	}
	byte = (uint8_t*)&net2;

	for(int j = 0; j < count.net2; j++)
	{
		for(size_t i  = 0; i < sizeof(net2); i++)
		{
			byteStream.push(byte[i]);
		}
	}
}

#define NETWORK_V1	0x01
#define NETWORK_V2	0x02

void dataParserThread(void *arg)
{
	Parser_Net1 parserNet1;
	Parser_Net2 parserNet2;
	uint8_t *netHeaderBufPtr = new uint8_t[sizeof(Net2_Header)]; /* Buff for read from queue */
	int netwDataReady = false;


	while(1)
	{
		if(readDataFromQueue(netHeaderBufPtr, 1))	/* read network ver byte */
		{
			if(netHeaderBufPtr[0] == NETWORK_V1)
			{
				while(!readDataFromQueue(&netHeaderBufPtr[1], sizeof(Net1_Header) - 1));
				parserNet1.parseHeader(netHeaderBufPtr, sizeof(Net1_Header));
			}
			else if(netHeaderBufPtr[0] == NETWORK_V2)
			{
				while(!readDataFromQueue(&netHeaderBufPtr[1], sizeof(Net2_Header) - 1));
				parserNet2.parseHeader(netHeaderBufPtr, sizeof(Net2_Header));
			}
			else
			{
				cout<<"Incorrect NetVer\n";
				break;
			}
		}
		else
		{
			static int counter = 0;
			counter ++;
			if(counter > 2000)
				break;
		}
	}

	cout<<"NetV1 packCount "<<parserNet1.getPackCount()<<" uniqIpCount "<<parserNet1.getUniqueIPcount()<<endl;
	cout<<"NetV2 packCount "<<parserNet2.getPackCount()<<" uniqIpCount "<<parserNet2.getUniqueIPcount()<<endl;

	delete []netHeaderBufPtr;
}

