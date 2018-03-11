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
#include <algorithm>
#include <vector>
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

struct Net1_Addr_t
{
	uint8_t byte[4];
public:
	bool operator==(const Net1_Addr_t &other)
	{
		return this->byte[0] == other.byte[0]
				&&this->byte[1] == other.byte[1]
				&&this->byte[2] == other.byte[2]
				&&this->byte[3] == other.byte[3];
	}
};

struct Net2_Addr_t
{
	uint8_t byte[6];
public:
	bool operator==(const Net2_Addr_t &other)
	{
		return this->byte[0] == other.byte[0]
				&&this->byte[1] == other.byte[1]
				&&this->byte[2] == other.byte[2]
				&&this->byte[3] == other.byte[3]
			    &&this->byte[4] == other.byte[4]
				&&this->byte[5] == other.byte[5];
	}
};

typedef struct
{
	Net1_Addr_t src;
	Net1_Addr_t dst;
}IP_Net_1_t;

typedef struct
{
	Net2_Addr_t src;
	Net2_Addr_t dst;
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
	IP_Net_2_t IPnet2;
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
	vector<Net1_Addr_t> uniqIPvect;	/* Contain unique IP addresses */

	int isUniqAddr(Net1_Addr_t addr)
	{
		int status = false;

		auto result1 = find(uniqIPvect.begin(), uniqIPvect.end(), addr);


		if (result1 != uniqIPvect.end())	/* address isn't unique */
		{
//			cout << "v содержит: " <<'\n';
		}
		else
		{
			status = true;
			cout << "v не содержит: " <<'\n';
		}

		return status;
	}

public:
	Parser_Net1()
	{
		protoType = 255;
		dataBSize = 0;
		packCount = 0;
		uniqIPcount = 0;
	};
	~Parser_Net1(){};

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

		if(isUniqAddr(headerBigEnd.IPnet1.src))
		{
			uniqIPcount++;
			uniqIPvect.push_back(headerBigEnd.IPnet1.src);//save unique addr
		}
		if(isUniqAddr(headerBigEnd.IPnet1.dst))
		{
			uniqIPcount++;
			uniqIPvect.push_back(headerBigEnd.IPnet1.dst);//save unique addr
		}

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
	vector<Net2_Addr_t> uniqIPvect;	/* Contain unique IP addresses */

	int isUniqAddr(Net2_Addr_t addr)
	{
		int status = false;

		auto result1 = find(uniqIPvect.begin(), uniqIPvect.end(), addr);


		if (result1 != uniqIPvect.end())	/* address isn't unique */
		{
//			cout << "v содержит: " <<'\n';
		}
		else
		{
			status = true;
			cout << "v не содержит: " <<'\n';
		}

		return status;
	}

public:
	Parser_Net2()
	{
		protoType = 255;
		dataBSize = 0;
		packCount = 0;
		uniqIPcount = 0;
	};
	~Parser_Net2(){};

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

		if(isUniqAddr(headerBigEnd.IPnet2.src))
		{
			uniqIPcount++;
			uniqIPvect.push_back(headerBigEnd.IPnet2.src);//save unique addr
		}
		if(isUniqAddr(headerBigEnd.IPnet2.dst))
		{
			uniqIPcount++;
			uniqIPvect.push_back(headerBigEnd.IPnet2.dst);//save unique addr
		}

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
	net2.IPnet2 = {0x12, 0x34, 0x56, 0x78, 0x90, 0x12};
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

