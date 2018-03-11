/*
 * netw_dump_parser.hpp
 *
 *  Created on: 5 марта 2018 г.
 *      Author: Slobodchikov RS
 */

#ifndef NETW_DUMP_PARSER_HPP_
#define NETW_DUMP_PARSER_HPP_


void queueWriterThread(void *arg);
void dataParserThread(void *arg);
void fileReaderThread(void *arg);

#endif /* NETW_DUMP_PARSER_HPP_ */
